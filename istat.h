#ifndef ELC_ISTAT_H
#define ELC_ISTAT_H

#define ELC_INT64_SIZ (64 / CHAR_BIT) /* eight for eight-bit char systems */
typedef unsigned char packed[ELC_INT64_SIZ];

typedef struct integrit_stat_struct {
  packed size;
  packed ino;
  packed atime;
  packed mtime;
  packed ctime;
  packed mode;
  packed nlink;
  packed uid;
  packed gid;
} istat_t;

void copy_sb_to_istat(istat_t *istat, struct stat *sb);
void copy_istat_to_sb(struct stat *sb, istat_t *istat);

#endif
