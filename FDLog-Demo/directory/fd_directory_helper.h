#ifndef fd_directory_helper_h
#define fd_directory_helper_h

/*
 * Function: fd_makedir
 * ----------------------------
 *
 *   path: dir path
 *
 *   returns: 0 or 1
 *
 */
int fd_makedir(const char *path);


/*
 * Function: fd_is_file_exist
 * ----------------------------
 *
 *   path: file path
 *
 *   returns: 0 or 1
 *
 */
int fd_is_file_exist(const char *path);


/*
 * Function: getmaximum
 * ----------------------------
 *
 *   a: all element of wait compare
 *   numberOfElements: numberOfElements
 *
 *   returns: the maximum of a[]
 *
 */
long getmaximum(long a[], int numberOfElements);

/*
 * Function: get_current_date
 * ----------------------------
 *
 *   returns: current date
 *
 */
char* get_current_date(void);

/*
 * Function: look_for_last_logfile
 * ----------------------------
 *
 *   returns: find last logfile name
 *
 */
char* look_for_last_logfile(void);

/*
 * Function: create_new_logfile
 * ----------------------------
 *
 *   returns: 0 or 1
 *
 */
int create_new_logfile(void);


#endif
