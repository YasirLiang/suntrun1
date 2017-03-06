/*
* @file
* @brief main function
* @ingroup main function
* @cond
******************************************************************************
* Last updated for version 1.0.0
* Last updated on  2016-09-22
*
*                    Moltanisk Liang
*                    ---------------------------
*                    avb auto control system
*
* Copyright (C) Moltanisk Liang, GuangZhou Suntron. All rights reserved.
******************************************************************************
* @endcond
*/
/*Including files-----------------------------------------------------------*/
#include "avdecc_main.h"
#include "avdecc_funhdl.h"
#include "avdecc_funhdl_native.h"
#include "entity.h"
#include "system.h"
#include "controller_command.h"
#include "send_pthread.h"
#include "check_timer.h"
#include "log_machine.h" /*log machine including file*/
#include "control_surface.h" /*menu control display*/
#include <signal.h>
#include <ucontext.h>
#include "controller_machine.h"
#include "raw_network.h"
#include "global.h"
/*Global varialable---------------------------------------------------------*/
struct fds net_fd;                               /*net communication fd set */
struct raw_context net;                               /* raw socket context */
struct socket_info_s pc_controller_server;            /* udp server context */
solid_pdblist endpoint_list; /*terminal double list guard node in the system*/
inflight_plist command_send_guard;       /* inflight double list guard node */
desc_pdblist descptor_guard;                 /* 1722.1 terminal description */
struct threads_info threads;                          /* system threads set */
volatile bool m_isRunning = 1;                                     /* system running flags */
/*Global varialable for log machine-----------------------------------------*/
char *glog_file_name = NULL;                         /* pointer to log file */
FILE *glog_file_fd = NULL;                                   /* log file fd */
char gmain_buf[2048] = {0};                              /* log file buffer */
/*Global varialable for dump core-----------------------------------------*/
int *gmain_stack_fp = NULL;                   /* main function's fp pointer */
int gdump_core_fd = -1;                                   /* dump record fd */
int gdump_core_flag = 0;                       /* dump flag for once record */
int gsig_num;                                           /* signation number */
siginfo_t gsegv_info;                     /* information of catching signal */
ucontext_t gsegv_ptr;                               /* context of signation */
/*Macro define for printf---------------------------------------------------*/
#define  debug_printf(format, args...) \
    do {\
        memset(gmain_buf, 0, sizeof(gmain_buf));\
        snprintf(gmain_buf,  100,  format, ##args);\
	w_printf(gmain_buf);\
    }while(0)
/*enable arm version if __ARM_BACK_TRACE__ is defined-----------------------*/
#ifdef __ARM_BACK_TRACE__
/*write information to file-------------------------------------------------*/
#define  w_printf(pt_str) \
    if ((gdump_core_flag == 0)\
          && (gdump_core_fd != -1))\
    {\
        write(gdump_core_fd, pt_str, strlen(pt_str));\
	printf("%s", pt_str);\
    }
/*Local function delaration-------------------------------------------------*/
static int backtrace_arm(int **buffer,  int size);
static void back_trace_dump(void);
static void sigsegv_handler(int signum, siginfo_t *si_info, void * ptr);
static void catch_sigsegv(void);
/*$ extern function declartion----------------------------------------------*/
extern int pthread_cli_create(pthread_t *cli_pid);
/*$ arm backtrace-----------------------------------------------------------*/
static int backtrace_arm(int ** buffer, int size) {
    int *fp = 0;
    int i  = 0;         /* Loop varialable */
    if (size <= 0) {	   /* Limit Check. */
        return 0;
    }

    /* get starting address of current sub-program information of
    pushing stack in Stack Frame(address is down growth)*/
    __asm__("mov %0, fp\n" : "=r"(fp) );
    /* get the subroutine return address at all level */
    while ((i < size)
                && (fp != NULL))
    {
        /* get current sub-program return address */
        buffer[i++] = (int *)(*(fp - 1));
        /* printf backstrace level */
        debug_printf("Backstrace Level %d = %p\r\n", i - 1, buffer[i - 1]);
        if ((buffer[i - 1] != NULL)
              && (fp != gmain_stack_fp))
        {
            /*get the value of saving in upper level FP register from
                Stack Frame*/
            fp =  (int *)(*(fp - 3));
            break;
        }
    }
    /* printf new line */
    debug_printf("\r\n");
    /* get stack deep level */
    if (i >= size) {
        i = size;
    }
    /* printf stack deep level */
    debug_printf("Backstrace Deep = %d\r\n",  i);
    /* printf new line */
    debug_printf("\r\n");
    return i;/* return stack deep level */
}
/*$ back_trace_dump---------------------------------------------------------*/
static void back_trace_dump(void)
{
    int * buffer[100];
    backtrace_arm(buffer, 100);/* get back information of stack */
}
/*$ sigsegv_handler---------------------------------------------------------*/
static void sigsegv_handler(int signum, siginfo_t *si_info, void *ptr) {
    ucontext_t * text = (ucontext_t *)ptr;
    static const char *si_codes[3] = {"",  "SEGV_MAPERR",  "SEGV_ACCERR"};

    gsig_num = signum;
    gsegv_info.si_errno = si_info->si_errno;
    gsegv_info.si_code = si_info->si_code;
    gsegv_info.si_addr = si_info->si_addr;

    gsegv_ptr.uc_mcontext.arm_fp = text->uc_mcontext.arm_fp;
    gsegv_ptr.uc_mcontext.arm_ip = text->uc_mcontext.arm_ip;
    gsegv_ptr.uc_mcontext.arm_sp = text->uc_mcontext.arm_sp;
    gsegv_ptr.uc_mcontext.arm_lr = text->uc_mcontext.arm_lr;
    gsegv_ptr.uc_mcontext.arm_pc = text->uc_mcontext.arm_pc;
    gsegv_ptr.uc_mcontext.arm_cpsr = text->uc_mcontext.arm_cpsr;

    /* open or create new file and reset the context in the file */
    if (gdump_core_flag == 0) {
        gdump_core_fd = open("dump_core.txt",
                                       O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
    }

    /* save happen time */ 
    time_t tem = time( NULL );
    struct tm *t = (struct tm*)localtime(&tem);/* get localtime */
    debug_printf("---------- Current Time = %d-%d-%d %d:%d:%d ----------\r\n",
                                                 t->tm_year+1900, t->tm_mon+1,
                                t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
    debug_printf("\r\n");

    debug_printf("Segmentation Fault Trace:\r\n");
    debug_printf("info.si_signo = %d\r\n", gsig_num);
    debug_printf("info.si_errno = %d\r\n", gsegv_info.si_errno);
    debug_printf("info.si_code  = %d (%s)\r\n", gsegv_info.si_code,
                                                si_codes[gsegv_info.si_code]);
    debug_printf("info.si_addr  = %p\r\n", gsegv_info.si_addr);
    debug_printf("\r\n");

    /* For ARM printf information */ 
    debug_printf("the arm_fp    = 0x%08x\r\n",
                                      (uint32_t)gsegv_ptr.uc_mcontext.arm_fp);
    debug_printf("the arm_ip    = 0x%08x\r\n",
                                      (uint32_t)gsegv_ptr.uc_mcontext.arm_ip);
    debug_printf("the arm_sp    = 0x%08x\r\n",
                                      (uint32_t)gsegv_ptr.uc_mcontext.arm_sp);
    debug_printf("the arm_lr    = 0x%08x\r\n",
                                      (uint32_t)gsegv_ptr.uc_mcontext.arm_lr);
    debug_printf("the arm_pc    = 0x%08x\r\n",
                                      (uint32_t)gsegv_ptr.uc_mcontext.arm_pc);
    debug_printf("the arm_cpsr  = 0x%08x\r\n",
                                    (uint32_t)gsegv_ptr.uc_mcontext.arm_cpsr);
    debug_printf("\r\n");

    /* stack information */
    back_trace_dump();
    /* dump core ? */
    if (gdump_core_flag == 0) { /* for the first time excute? */
        if (gdump_core_fd != -1) {        /* open successs? */
            fsync( gdump_core_fd );      /* syn,write to file */
            close( gdump_core_fd );             /* close file */
            gdump_core_fd   = -1;                    /* clear */
            gdump_core_flag = 1;/* make sure not excute again */
        }
    }
}
/*$ catch_sigsegv-----------------------------------------------------------*/
static void catch_sigsegv(void) {/* signal segv handle function register */
    struct sigaction sa_segv;
    memset(&sa_segv, 0, sizeof(struct sigaction));
    sa_segv.sa_flags = SA_SIGINFO;
    sa_segv.sa_sigaction = sigsegv_handler;  /* signal segv handle function */
    if (sigaction(SIGSEGV, &sa_segv, NULL ) == -1) {     /*register SIGSEGV */
        perror( "sigaction: " );
        exit( -1 );
    }
}
#endif /*__ARM_BACK_TRACE__*/
/*Local function delaration-------------------------------------------------*/
/* C-c signal Local signal delaration --------------------------------------*/
static void catch_sigin(void);
/* log_callback_func delaration --------------------------------------------*/
static void log_callback_func(void *user_obj, int32_t log_level,
                                      const char *msg, int32_t time_stamp_ms);
/* C-c handle function delaration ------------------------------------------*/
static void signal_handle_main(int signum);
/*$ signal_handle_main -----------------------------------------------------*/
static void signal_handle_main(int signum) {
    if (SIGINT == signum) {                /* C-c signal? */
        system_close(&threads); /* system close, clear... */
        exit(0); /*exit proccess*/
    }
}
/*$ catch_sigin ------------------------------------------------------------*/
static void catch_sigin(void) {
    struct sigaction sa;
    sa.sa_handler = signal_handle_main;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    /* register C-c signal handle function */
    if (sigaction( SIGINT, &sa, NULL ) == -1) {
        perror("sigaction: ");
        exit(-1);
    }
}
/*$ log_callback_func ------------------------------------------------------*/
static void log_callback_func(void *user_obj, int32_t log_level,
                                       const char *msg, int32_t time_stamp_ms)
{
    if( glog_file_fd == NULL ) {
        /* printf to screen */ 
        printf( "[LOG] %s %s\n", logging_level_string_get(log_level), msg);
    }
    else { /* log to file */ 
        time_t tem = time(NULL);
        struct tm *t = (struct tm*)localtime(&tem);/* get local time */
        /* first reset buffer */
        memset(gmain_buf, 0, sizeof(gmain_buf));
        /* format new line information */
        sprintf(gmain_buf, "[%d-%d-%d %d:%d:%d LOG] %s  %s\n",
                t->tm_year+1900, t->tm_mon+1, t->tm_mday, t->tm_hour,
                t->tm_min, t->tm_sec,
                logging_level_string_get(log_level), msg);
        fputs(gmain_buf, glog_file_fd);/* write to file */
        Fflush(glog_file_fd); /*flush buffer to file */
    }
}

int main(int argc, char *argv[]) {
/*enable arm version if __ARM_BACK_TRACE__ is defined-----------------------*/
#ifdef __ARM_BACK_TRACE__
    __asm__( "mov %0, fp\n" : "=r"(gmain_stack_fp));
#endif /*__ARM_BACK_TRACE__*/
    int32_t log_level = LOGGING_LEVEL_ERROR;/* default level is error */
    fprintf(stdout, "Usage: ./programing 0/1/2/3/4/5(log level)"
                                          "file_name(none log to screen) \n");
    if (argc >= 2) {
        log_level = atoi(argv[1]);/* get log level */
    }
    if (log_level >= TOTAL_NUM_OF_LOGGING_LEVELS) {/* out of blank? */
        log_level = LOGGING_LEVEL_ERROR;
    }

    if (argc >= 3) {
        glog_file_name = argv[2];/* get log file name */
        glog_file_fd = Fopen(glog_file_name, "w+");/*open log file*/
        if (glog_file_fd != NULL) {
            fprintf(stdout, "your log message file Name is %s,"
                                           "Open Success!\n", glog_file_name);
        }
        else {
            fprintf(stdout, "your log message file Name is %s,"
                               "Open failed, Check Right!\n", glog_file_name);
            glog_file_fd = NULL;
        }
    }
    /* printf information of log level and whether log to file? */
    fprintf(stdout, "Will Usage: %s message can be logged,"
                                           "and only log to screen? %s\n\r\n",
                                         logging_level_string_get(log_level),
                                         (glog_file_name == NULL)?"YES":"NO");
    /* log machine create */
    if (NULL == log_machine_create(log_callback_func, log_level, NULL)) {
        fprintf(stdout, "your system can't log massge,"
                                      "log machine class create is failed\n");
    }
    /* controller machine create */
    gp_controller_machine = controller_machine_create();
    if (NULL != gp_controller_machine) {/* create success? */
        int i = 0;          /* loop variable */
        uint64_t t;         /* mac one bytes */
        void *p; /* user object void pointer */
        raw_net_1722_user_info* raw_user_obj;/* user object pointer */
        controller_machine_init(gp_controller_machine, raw_network_init,
                     raw_network_send, raw_network_recv, raw_network_cleanup);
        /*get user object of 1722 network */
        p = gp_controller_machine->unit_1722_net->network_1722_user_obj;
        if (p != NULL) {
            /* set user information, and get port information */
            raw_user_obj = (raw_net_1722_user_info*)p;
            /* copy index id */
            net.m_interface_id = raw_user_obj->ifindex;
            /* copy mac address */
            for (i = 0; i < 6; ++i) {
                t = (raw_user_obj->mac >> ((5-i)*8)) & 0x00000000000000ff;
                net.m_my_mac[i] = (uint8_t)t;
            }
            /* set network protocal type */
            net.m_ethertype = raw_user_obj->ethertype;
            /* set raw fd */
            net.m_fd = raw_user_obj->rawsock;
            /* set raw fd */
            net_fd.raw_fd = raw_user_obj->rawsock;
            /*set muticastor dest mac address*/
            memcpy(net.m_default_dest_mac,
                jdksavdecc_multicast_adp_acmp.value, 6);
        }
        else {/* exit proccess*/
            printf("1722_user_obj Info Error,Exit!\n");
            exit(-1);
        }
    }
    else {/* exit proccess*/
        printf("create_controller machine Error,Exit!\n");
        exit(-1);
    }
    /*register signal of C-c*/
    catch_sigin();
/*enable arm version if __ARM_BACK_TRACE__ is defined-----------------------*/
#ifdef __ARM_BACK_TRACE__
    /*register signal of sev*/
    catch_sigsegv();
#endif/*__ARM_BACK_TRACE__*/
    struct udp_context udp_net;              /* udp context */
    pthread_t h_thread;          /* I/O complicating thread */
    pthread_t proccess_thread;/* recieve data handle thread */
    pthread_t f_thread;          /* function command thread */
    pthread_t s_thread;            /* port data send thread */
    pthread_t c_thread;            /* command line thread */

    /* initial system */
    init_system();
    /* init network card */
    AvdeccNet_init(NETWORT_INTERFACE);
    /* system build socket */
    AvdeccNet_buildSocket(&net_fd, &net, NETWORT_INTERFACE, &udp_net);

    /* I/O complicating thread create */
    threads.pthread_nums = 0;/* reset thread num to zero */
    pthread_handle_create(&h_thread, &net_fd);
    threads.tid[threads.pthread_nums++] = h_thread;   /* save thread handle */
    pthread_detach(h_thread);             /* detach I/O complicating thread */
    
    /* recieve data handle thread create */
    pthread_proccess_recv_data_create(&proccess_thread, NULL);
    threads.tid[threads.pthread_nums++] = proccess_thread;     /*save handle*/
    pthread_detach(proccess_thread);   /* detach recieve data handle thread */
    
    /* function command thread create */
    pthread_handle_cmd_func(&f_thread, proccess_func_link_tables);
    threads.tid[threads.pthread_nums++] = f_thread;   /* save thread handle */
    pthread_detach(f_thread);             /* detach function command thread */
    
    /* port data send thread create */
    pthread_send_network_create(&s_thread);
    threads.tid[threads.pthread_nums++] = s_thread;   /* save thread handle */
    pthread_detach(s_thread);               /* detach port data send thread */

    pthread_cli_create(&c_thread);
    threads.tid[threads.pthread_nums++] = s_thread;   /* save thread handle */
    pthread_detach(s_thread);               /* detach thread */
    
    /* set system information */
    set_system_information(net_fd, &udp_net);
    pthread_exit(NULL);
}

