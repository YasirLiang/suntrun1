#include "conference.h"

inline ssize_t conference_validata_range(size_t bufpos, ssize_t cdata_len, size_t buflen)
{
	return ((ssize_t)bufpos + cdata_len) <= (ssize_t)buflen?((ssize_t)bufpos + (ssize_t)cdata_len): (sizeof(ssize_t) - 1);
}
