#include "avdecc_main.h"
#include "avdecc_funhdl.h"
#include "avdecc_funhdl_native.h"
#include "entity.h"
#include "system.h"
#include "controller_command.h"
#include "send_pthread.h"
#include "check_timer.h"
#include "log_machine.h"// 日志记录机制头文件
#include "control_surface.h" // 界面控制显示
#include <signal.h>
#include <ucontext.h>

struct fds net_fd;					// 网络通信套接字与线程间通信套接字
struct raw_context net;				// 原始套接字
struct socket_info_s pc_controller_server;	// 主机显示信息与摄像头控制器的通信信息 
solid_pdblist endpoint_list;			// 系统中终端链表哨兵节点
inflight_plist command_send_guard;	// 系统中发送网络数据命令链表哨兵节点
desc_pdblist descptor_guard;	// 系统中描述符链表哨兵节点
struct threads_info threads;

char *glog_file_name = NULL;// 日志文件名字指针
FILE *glog_file_fd = NULL;	// 日志文件描述符
char gmain_buf[2048] = {0};	// 日志文件的缓存

int *gmain_stack_fp = NULL;	// 主函数的fp指针
int gdump_core_fd = -1;		// 段错误文件描述符
int gdump_core_flag = 0;		// 只记录一次段错误的标志
int gsig_num;			// 信号
siginfo_t     gsegv_info;		// 信号的信息
ucontext_t    gsegv_ptr;		// 上下文

#define  w_printf( pt_str ) if( (gdump_core_flag == 0) && (gdump_core_fd != -1) ) { write(gdump_core_fd,  pt_str,  strlen( pt_str ) );\
						printf( "%s", pt_str);}
#define  debug_printf( format, args... )	do{memset(gmain_buf, 0, sizeof(gmain_buf)); snprintf( gmain_buf,  100,  format, ##args );\
									w_printf( gmain_buf );}while(0)
	
#ifdef __TEST_DEBUG_CM__ 
int thread_test_fn( void*pgm )
{
	uint64_t tar_id = 0;
	
	while( 1 )
	{
		sleep(1);
		terminal_set_mic_status( 1, 0, tar_id );
		sleep(1);
		terminal_set_mic_status( 0, 0, tar_id );
	}
}
#endif

void signal_handle_main( int signum )
{
	if( SIGINT == signum )
	{
		DEBUG_INFO( "System Close......" );
		system_close( &threads );
		DEBUG_INFO( "System Close Success!" );
		exit(0);
	}
}

int backtrace_arm(int ** buffer,  int size)
{
	int *fp = 0;
	int i  = 0;

	// Limit Check.
	if(size <= 0)	return 0;

	/*=============================================*/
	/*= 获取各级子程序的返回地址 ==================*/
	/*=============================================*/

	// 汇编获取 FP(R11) 寄存器的值( 当前子程序的入栈信息在 Stack Frame 中起始地址 --- 地址是向下生长的 ).
	__asm__("mov %0, fp\n" : "=r"(fp) );

	while((i < size)  &&  (fp != NULL))
	{
		buffer[ i++ ]  =  (int *)(* (fp - 1));	// 获取 Stack Frame 中保存的  链接寄存器 LR   的值( 当前子程序的返回地址 )
		debug_printf("Backstrace Level %d = %p\r\n", i - 1, buffer[i - 1]);

		if((buffer[i - 1] != NULL) && (fp != gmain_stack_fp))
		{
			fp =  (int *)(*(fp - 3)); // 获取 Stack Frame 中保存的  上一级FP寄存器  的值( 当前子程序的被调函数的入栈信息在 Stack Frame 中的起始地址 )
		}
		else
		{
			break;
		}
	}

	debug_printf("\r\n");

	/*=============================================*/
	/*= 返回堆栈深度 ==============================*/
	/*=============================================*/
	if(i >= size)	i = size;

	debug_printf("Backstrace Deep = %d\r\n",  i);
	debug_printf("\r\n");
	return i;
}


void back_trace_dump(void)
{
	int * buffer[100];

	backtrace_arm(buffer, 100);// 获取栈回溯的信息

	return;
}

void sigsegv_handler( int signum, siginfo_t *si_info, void * ptr )
{
	ucontext_t  * text = (ucontext_t * )ptr;
	static const char * si_codes[3] = { "",  "SEGV_MAPERR",  "SEGV_ACCERR" };

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

	// 打开或新建一个文件，并清空文件内容.
	if(gdump_core_flag == 0)
	{
		gdump_core_fd = open("dump_core.txt",  O_WRONLY | O_CREAT | O_TRUNC,  S_IRWXU );
	}

	// save happen time
	time_t tem = time( NULL );
	struct tm *t = (struct tm*)localtime( &tem );
	debug_printf("---------- Current Time = %d-%d-%d %d:%d:%d ----------\r\n", t->tm_year+1900,\
				t->tm_mon+1, t->tm_mday, t->tm_hour, t->tm_min,\
				t->tm_sec );
	debug_printf("\r\n");

	debug_printf("Segmentation Fault Trace:\r\n");
	debug_printf("info.si_signo = %d\r\n", gsig_num);
	debug_printf("info.si_errno = %d\r\n", gsegv_info.si_errno);
	debug_printf("info.si_code  = %d (%s)\r\n", gsegv_info.si_code,	si_codes[gsegv_info.si_code]);
	debug_printf("info.si_addr  = %p\r\n", gsegv_info.si_addr);
	debug_printf("\r\n");

	// For ARM.
	debug_printf("the arm_fp    = 0x%08x\r\n", (uint32_t)gsegv_ptr.uc_mcontext.arm_fp);
	debug_printf("the arm_ip    = 0x%08x\r\n", (uint32_t)gsegv_ptr.uc_mcontext.arm_ip);
	debug_printf("the arm_sp    = 0x%08x\r\n", (uint32_t)gsegv_ptr.uc_mcontext.arm_sp);
	debug_printf("the arm_lr    = 0x%08x\r\n", (uint32_t)gsegv_ptr.uc_mcontext.arm_lr);
	debug_printf("the arm_pc    = 0x%08x\r\n", (uint32_t)gsegv_ptr.uc_mcontext.arm_pc);
	debug_printf("the arm_cpsr  = 0x%08x\r\n", (uint32_t)gsegv_ptr.uc_mcontext.arm_cpsr);
	debug_printf("\r\n");

	// 堆栈信息.
	back_trace_dump();

	/**
	*返回
	*/
	if(gdump_core_flag == 0)// 首次执行?
	{
		// 已成功打开.
		if(gdump_core_fd != -1)
		{
			fsync( gdump_core_fd );// 同步，将文件缓冲区中的数据写入磁盘文件中.
			close( gdump_core_fd );// 关闭文件.
			gdump_core_fd   = -1;// Clear.
			gdump_core_flag = 1;// 执行完毕，只执行一次.
		}
	}
}

void catch_sigin( void )
{
	struct sigaction sa;
	sa.sa_handler = signal_handle_main;
	sigemptyset( &sa.sa_mask );
	sa.sa_flags = 0;
	if( sigaction( SIGINT, &sa, NULL ) == -1 )// 注册ctl+c 处理函数
	{
		perror( "sigaction: " );
		exit( -1 );
	}
}

void catch_sigsegv( void )
{
	struct sigaction sa_segv;
	memset( &sa_segv, 0, sizeof(struct sigaction));
	sa_segv.sa_flags = SA_SIGINFO;
	sa_segv.sa_sigaction = sigsegv_handler;
	if( sigaction( SIGSEGV, &sa_segv, NULL ) == -1 )// 注册段违规信号处理函数
	{
		perror( "sigaction: " );
		exit( -1 );
	}
}

void log_callback_func(void *user_obj, int32_t log_level, const char *msg, int32_t time_stamp_ms )
{
	if( glog_file_fd == NULL )
	{// printf to screen
		printf( "[LOG] %s (%s)\n", logging_level_string_get(log_level), msg );
	}
	else
	{// log to file
		time_t tem = time( NULL );
		struct tm *t = (struct tm*)localtime( &tem );
		memset( gmain_buf, 0, sizeof(gmain_buf) );
		sprintf( gmain_buf, "[%d-%d-%d %d:%d:%d LOG] %s  (%s)\n", t->tm_year+1900,\
				t->tm_mon+1, t->tm_mday, t->tm_hour, t->tm_min,\
				t->tm_sec,
				logging_level_string_get(log_level),
				msg );
		
		fputs( gmain_buf, glog_file_fd );
		Fflush( glog_file_fd );
	}
}

int main( int argc, char *argv[] )
{
	__asm__( "mov %0, fp\n" : "=r"(gmain_stack_fp));
	
	int32_t log_level = LOGGING_LEVEL_ERROR;
	
	fprintf( stdout, "Usage: ./programing 0/1/2/3/4/5(log level) file_name(none log to screen) \n");
	if( argc >= 2 )
		log_level = atoi(argv[1]);
	if( log_level >= TOTAL_NUM_OF_LOGGING_LEVELS )
		log_level = LOGGING_LEVEL_ERROR;
	fprintf( stdout, "Will Usage: %s message can be logged, and only log to screen\n", logging_level_string_get(log_level) );
	if( argc >= 3 )
	{
		glog_file_name = argv[2];
		glog_file_fd = Fopen( glog_file_name, "a+" );
		if( glog_file_fd != NULL )
		{
			fprintf( stdout, "your log message file Name is %s, Open Success!\n", glog_file_name );
		}
		else 
		{
			fprintf( stdout, "your log message file Name is %s, Open failed, Check Right!\n", glog_file_name );
			glog_file_fd = NULL;
		}
	}

	if( NULL == log_machine_create( log_callback_func, log_level, NULL ) )
	{
		fprintf( stdout, "your system cant log massge, log machine class create is failed\n" );
	}

	catch_sigin();
	catch_sigsegv();

	struct udp_context udp_net;
	init_system();	// 初始化系统，包括系统管理终端链表,inflight命令链表
	build_socket( &net_fd, &net, NETWORT_INTERFACE, &udp_net ); // 建立raw, udp server, udp client socket
	build_pipe( net_fd.tx_pipe );	// 创建无名管道

	threads.pthread_nums = 0;
	pthread_t h_thread;
	pthread_handle_create( &h_thread, &net_fd ); // 创建接收数据处理线程
	threads.tid[threads.pthread_nums++] = h_thread;
	pthread_detach( h_thread );
	
	pthread_t proccess_thread;
	pthread_proccess_recv_data_create( &proccess_thread, NULL );
	threads.tid[threads.pthread_nums++] = proccess_thread;
	pthread_detach( proccess_thread );
	
#if 0
	pthread_t p_thread;
	pthread_handle_pipe( &p_thread, &net_fd ); // 创建读管道与发送网络数据线程
	threads.tid[threads.pthread_nums++] = p_thread;
	pthread_detach( p_thread );
#endif

	pthread_t f_thread;	// 接收命令处理函数
	pthread_handle_cmd_func( &f_thread, proccess_func_link_tables );
	threads.tid[threads.pthread_nums++] = f_thread;
	pthread_detach( f_thread );
	
#ifndef __NOT_USE_SEND_QUEUE_PTHREAD__
	pthread_t s_thread; // 发送网络数据的线程
	pthread_send_network_create( &s_thread );
	threads.tid[threads.pthread_nums++] = s_thread;
	pthread_detach( s_thread );
#endif
	
#ifdef __TEST_DEBUG_CM__
	pthread_t test_cm_thread; // 测试线程
	int rc = 0;
	rc = pthread_create( &test_cm_thread, NULL, (void*)&thread_test_fn, NULL );
	if( rc )
	{
		DEBUG_INFO(" test_cm_thread ERROR; return code from pthread_create() is %d\n", rc);
		assert( rc == 0 );
	}

	threads.tid[threads.pthread_nums++] = test_cm_thread;
	pthread_detach( test_cm_thread );
#endif

#if 0
	pthread_t timer_pthread;
	check_timer_create( &timer_pthread );
#endif

#if 0
	/**
	*创建界面处理线程
	*/
	pthread_t th;  
	int ret;  
	ret = pthread_create( &th, NULL, thread_control_surface, NULL);  
	if( ret != 0 ){  
		printf( "Create thread_control_surface error!\n");  
		exit(1); 
	}
	threads.tid[threads.pthread_nums++] = th;
	pthread_detach( th );// 与主线程分离
#endif
	DEBUG_ONINFO("waiting for endpoint for connect!");
	set_system_information( net_fd, &udp_net );

	controller_proccess();

	pthread_exit( NULL );
}

