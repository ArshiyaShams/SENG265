#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define MAX_LINE_LEN 132
#define MAX_EVENTS 500

/* typdef struct to group together the data from the event type */
typedef struct event{
    char dt_start[MAX_LINE_LEN];
    char dt_end[MAX_LINE_LEN];
    char dt_repeat[MAX_LINE_LEN];
    char location[MAX_LINE_LEN];
    char summary[MAX_LINE_LEN];
    char *ampm_start;
    char *ampm_end;
} event;

event arr_events[MAX_EVENTS];   //global variable for storing events from our read function

/*  putting this here so i can do my actual work with these functions under main (the ones i dont call
*   in main i do not need here, but i put them in just to be safe)
*/
void dt_format(char *formatted_time, const char *dt_time, const int len);
void dt_increment(char *after, const char *before, int const num_days);
int compare_event(const void *x, const void *y);
int read(const char *filename, int date_from, int date_to, int count);
void print_events(int count);
void convert(int from_y, int from_m, int from_d, int to_y, int to_m, int to_d, int *start, int *end);

int main(int argc, char *argv[]){

    int from_y = 0, from_m = 0, from_d = 0;
    int to_y = 0, to_m = 0, to_d = 0;
    char *filename = NULL;
    int i;
    
    for (i = 0; i < argc; i++) {
        if (strncmp(argv[i], "--start=", 8) == 0) {
            sscanf(argv[i], "--start=%d/%d/%d", &from_y, &from_m, &from_d);
        } else if (strncmp(argv[i], "--end=", 6) == 0) {
            sscanf(argv[i], "--end=%d/%d/%d", &to_y, &to_m, &to_d);
        } else if (strncmp(argv[i], "--file=", 7) == 0) {
            filename = argv[i]+7;
        }
    }

    if (from_y == 0 || to_y == 0 || filename == NULL) {
        fprintf(stderr, 
            "usage: %s --start=yyyy/mm/dd --end=yyyy/mm/dd --file=icsfile\n",
            argv[0]);
        exit(1);
    }

    /* Starting calling your own code from this point. */
    int count = 0; //tracks number of events
    int starting_date, end_date; //holds the start and end from the comman line
    convert(from_y, from_m, from_d, to_y, to_m, to_d, &starting_date, &end_date);
    count = read(filename, starting_date, end_date, count);
    print_events(count);
    exit(0);
}

/*
*
*    dt_format function from the timeplay.c
*    creates a more readable version of the date
*    for example:
*    
*    20190520T111500
*
*    and the "formatted_time" is:
*
*   May 20, 2019 (Mon).
*
*
*/

void dt_format(char *formatted_time, const char *dt_time, const int len){

    struct tm temp_time;
    time_t  full_time;
    char    temp[5];

    memset(&temp_time, 0, sizeof(struct tm));
    sscanf(dt_time, "%4d%2d%2d", &temp_time.tm_year, &temp_time.tm_mon, &temp_time.tm_mday);
    temp_time.tm_year -= 1900;
    temp_time.tm_mon  -= 1;
    full_time = mktime(&temp_time);
    strftime(formatted_time, len, "%B %d, %Y (%a)", localtime(&full_time));

}

/*
*   dt_increment from timeplay.c
*   when given a date-time, adds the number of days
*   in a way that it results in the correct new date
*   for example:
*   
*   20190520T111500
*
*   You get (assumming that num_days is 100):
*
*   20190828T111500
*
*   which is 100 days after our initial date
*
*/

void dt_increment(char *after, const char *before, int const num_days){

    struct tm   temp_time, *p_temp_time;
    time_t      full_time;
    char        temp[5];

    memset(&temp_time, 0, sizeof(struct tm));
    sscanf(before, "%4d%2d%2d", &temp_time.tm_year, &temp_time.tm_mon, &temp_time.tm_mday);
    temp_time.tm_year -= 1900;
    temp_time.tm_mon  -= 1;
    temp_time.tm_mday += num_days;

    full_time = mktime(&temp_time);
    after[0] = '\0';
    strftime(after, 9, "%Y%m%d", localtime(&full_time));
    strncpy(after + 8, before + 8, MAX_LINE_LEN - 8);
    after[MAX_LINE_LEN - 1] = '\0';
}

/*
Point: This is called from qsort in the read function
       and used to sort events by date by taking two void
       pointers x and y
*/

int compare_event(const void *a, const void *b){

    /* Converts pointer to pointer to int */
    event *ea = (event *)a;
    event *eb = (event *)b;

    /* If a positive result, then x>y    
       If a negative result, then y>x
       If a zero is the result, then x = y 
    */
    return strcmp(ea -> dt_start, eb -> dt_start);

}


/*
Point: opens and reads a file line by line and stores the values of the ics file.
       into a struct. ALso determines if any repeating events an how many of them.
       If there are multiple events it puts them into qsort() and sorts them.
*/

int read(const char *filename, int start, int end, int count){

    char line[MAX_LINE_LEN];
    FILE* infile = fopen(filename, "r");

    /* If the file is unopenable then you exit */
    if(infile == NULL){
        exit(1);
    }

    /* Reads through every line of the file till the end */
    while (fgets(line, MAX_LINE_LEN, infile)){
        /* Determines where to start taking the event info  */
        if (strncmp(line, "BEGIN:VEVENT", 12) == 0){
            event cur_event;    //store events information

        /* If the property is found, scan and store the value: <property>:<value>  */
        /* Finds the line "DTSTART:"  */
        while(strncmp(line, "DTSTART:", 8) != 0){        
            fgets(line, MAX_LINE_LEN, infile);
        }
        sscanf(line, "DTSTART:%s", cur_event.dt_start);

        /* Finds the line "DTEND:"  */
        while(strncmp(line, "DTEND:", 6) != 0){
            fgets(line, MAX_LINE_LEN, infile);
        }
        sscanf(line, "DTEND:%s", cur_event.dt_end);
        
        fgets(line, MAX_LINE_LEN, infile);
        /* check if event repeats by if file has "RRULE"  */
        if(strncmp(line, "RRULE", 5) ==0){
                 
            if(strncmp(line, "RRULE:FREQ=WEEKLY;WKST=MO;UNTIL=", 32) == 0){
                sscanf(line, "RRULE:FREQ=WEEKLY;WKST=MO;UNTIL=%15s", cur_event.dt_repeat);
            } else if (strncmp(line, "RRULE:FREQ=WEEKLY;UNTIL=", 24) == 0){
                sscanf(line, "RRULE:FREQ=WEEKLY;UNTIL=%15s", cur_event.dt_repeat);
            }
            fgets(line, MAX_LINE_LEN, infile);
        } else {
            strncpy(cur_event.dt_repeat, cur_event.dt_start, MAX_LINE_LEN);
        } 
          /* Finds the line with the LOCATION propert */
          while (strncmp(line, "LOCATION", 8) != 0){
            fgets(line, MAX_LINE_LEN, infile);        
        }
        /*Store all the location info from the property */
        strncpy(cur_event.location, &line[9], strlen(line));
        cur_event.location[strlen(cur_event.location) - 1] = '\0';
        
        /* Finds the line with SUMMARY property */
        while(strncmp(line, "SUMMARY", 7) != 0){
            fgets(line, MAX_LINE_LEN, infile);
        }
        /* store all the summary info from the property */
        strncpy(cur_event.summary, &line[8], strlen(line));
        cur_event.summary[strlen(cur_event.summary) -1] = '\0';

        /*
        Finds out if start and end date are within the command line argument
        If it does then you strore it into the global array arr_events[] and 
        brings up the number of events
        */
        if ((atoi(cur_event.dt_start) >= start) && (atoi(cur_event.dt_end) <= end)){
            arr_events[count] = cur_event;
            count++;
        }
        /*  Finds number of times the event repeats */
        while(strncmp(cur_event.dt_start, cur_event.dt_repeat, 15) < 0){
            char temp[MAX_LINE_LEN];
            /*
            increments the start and end date by a week
            then puts the new time temp char array into the 
            events start and end
            */
            dt_increment(temp, cur_event.dt_start, 7);
            strncpy(cur_event.dt_start, temp, 15);  
            dt_increment(temp, cur_event.dt_end, 7);
            strncpy(cur_event.dt_end, temp, 15);
           
            /*
            Start and end date of the file have to fall in line
            with the dates from the command line. If equal to our repeat until date,
            we add the event into the array and increase the count
            */
            if (((atoi(cur_event.dt_start) >= start) && (atoi(cur_event.dt_end) <= end)) && (strncmp(cur_event.dt_start, cur_event.dt_repeat, 15) <= 0)){
                arr_events[count] = cur_event;
                count++;
            }
        }      
    } 
 }
    fclose(infile);
    qsort(arr_events, count, sizeof(event), compare_event);

    return count;
}

/*
Point: this is here to print out all of the events.
       It has a more readable output since it call our
       format function and it also turns out time from 24 hour to 12
       and makes sure it has the appropriate AM or PM tag too.
*/
void print_events(int count){

    int hour_start, minute_start;
    int hour_end, minute_end;
    
    /* Loops through the number of events */
    for(int i=0; i<count; i++){
    
        sscanf(arr_events[i].dt_start, "%*8dT%2d%2d", &hour_start, &minute_start);
        sscanf(arr_events[i].dt_end, "%*8dT%2d%2d", &hour_end, &minute_end);
        // start time
        if(hour_start < 12){
            arr_events[i].ampm_start = "AM";
        } else if (hour_start == 12){
            arr_events[i].ampm_start = "PM";
        } else {
            arr_events[i].ampm_start = "PM";
            hour_start -= 12;
        }
        // end time
        if(hour_end < 12){
            arr_events[i].ampm_end = "AM";
        } else if(hour_end == 12) {
            arr_events[i].ampm_end = "PM";
        } else {
            arr_events[i].ampm_end = "PM";
            hour_end -= 12;
        }        

        /* Determines if a new day has to be printed, since if an event takes place on the same day we dont need to reprint it */
        if((i==0) || ((i>0) && (strncmp(arr_events[i].dt_start, arr_events[i-1].dt_start, 8) != 0))){
        
            if ( i > 0 ) {
                printf("\n");
            }
            
            char formatted_date[MAX_LINE_LEN];
            dt_format(formatted_date, arr_events[i].dt_start, MAX_LINE_LEN);
            printf("%s\n", formatted_date);

            for(int j = 0; j < strlen(formatted_date); j++){
                printf("-");
            }        
            printf("\n");
        }
        
        printf("%2d:%02d %s to %2d:%02d %s: %s {{%s}}\n",
              hour_start, minute_start, arr_events[i].ampm_start,
              hour_end, minute_end, arr_events[i].ampm_end, arr_events[i].summary, arr_events[i].location);
    }
}

void convert(int from_y, int from_m, int from_d, int to_y, int to_m, int to_d, int *start, int *end){
    *start = from_y * 10000 + from_m * 100 + from_d;
    *end = to_y * 10000 + to_m * 100 + to_d;
}








