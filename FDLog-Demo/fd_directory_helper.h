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
 * Function: create_new_current_date_logfile
 * ----------------------------
 *
 *   returns: 0 or 1
 *
 */
int create_new_current_date_logfile(void);

/*
 * Function: create_new_logfile
 * ----------------------------
 *
 *   date: create date file
 *
 *   returns: 0 or 1
 *
 */
//int create_new_logfile(char* date);

/*
 * Function: remove_log_file
 * ----------------------------
 *
 *   save_recent_days_num: save recent days
 *
 */
void remove_log_file(int save_recent_days_num, char* root_path);

/*
 * Function: remove_file
 * ----------------------------
 *
 *   path: file path
 *
 */
void remove_file(char* path);


#endif
