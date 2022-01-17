#!/usr/bin/env python3

import datetime
import argparse

"""

Helper Functions:

"""

def ical_object(ical_date):
    return datetime.datetime(int(ical_date[0:4]), int(ical_date[4:6]), int(ical_date[6:8]), int(ical_date[9:11]),
                             int(ical_date[11:13]), int(ical_date[13:]))
# a section of this comes from a stackoverflow post oart used linked below
# tinyurl.com/5dtb48mm
def cmdl_object(cmdl_date):
    i = [pos for (pos, word) in enumerate(cmdl_date) if word == '/']
    return datetime.date(int(cmdl_date[:i[0]]), int(cmdl_date[(i[0] + 1):i[1]]), int(cmdl_date[(i[1]+1):]))
# This gives you a string version of the month from the date object
# and we want the full thing so we use %B
def month(date):
    return date.strftime("%B")
# This gives you a string version of the day of the week of our date object
# and since we only want the first 3 letters i.e.m "Sat" we use %a
def day_of(date):
    return date.strftime("%a")
# Sorts our events by date
#   sorting algo borrowed from tinyurl.com/n8jmcypn (from geeksforgeeks)
def sorter(events):
    for date in events:
        events[date].sort(key = lambda time: [time[0]])
    return events
# you get a string representaiont of the time
# 24 hour time to 12 hour time with an AM or a PM
# i.e., 13:15:00 -> 1:30 PM
def hours(time):
    return time.strftime("%_I:%M %p")
# Does what the name of the function says
# It moves up the date by a week
def increment_by_week(date):
    return date + datetime.timedelta(weeks = 1)


"""

Main Functions:

"""

"""
Point: storing our events into a dictionary

"""
def store(groups):
    event = {}
    for group in groups: 
        # we want to find out if our event repeates
        # the if and elif can be changed with a little bit of code (changing out the slices pretty much)
        # if is for UNTILs that come earlier and elif is for ones that come later
        # the else is for no repeation so we take group 1 which is DTEND
        if(group[2][18:23] == 'UNTIL'):
            date_until = group[2][24:39]
        elif(group[2][0] == 'R'):
            date_until = group[2][32:47]
        else:
            date_until = group[1][6:]            
        event = store_rep(event, ical_object(group[0][8:]), ical_object(group[1][6:]), ical_object(date_until), group[-1][8:], group[-2][9:])
    events = sorter(event)
    return event

"""

Point: Storing repeated events into a dictionary by date

"""
def store_rep(event, start, end, until, sumry, loc):
    # you want to repeate till end date is less than the UNTIL date
    while end <= until:
        date = start.date()
        if date not in event:
            event[date] = []
        event[date].append((start.time(), end.time(), sumry, loc))
        start = increment_by_week(start)
        end = increment_by_week(end)
    return event

"""
Point:      To open & read the file. It puts each of the events into a group of its own
Parameters: filename --- our ics file
returned:   Events   --- collection of the events
"""
def read(filename):
    try:
        # open your file and strip the newline character from the lines
        with open(filename) as infile:
            lines = [line.strip() for line in infile]
        # removes any lines we do not need
        lines = [line for line in lines if line not in ('BEGIN:VCALENDAR', 'END:VCALENDAR',) 
                if (line[0:8] != 'VERSION:')]

        # splits up the file into groups
        groups = ('\n'.join(lines)).split('END:VEVENT')
        # removes empty groups
        groups.remove('')
        # split the lines in the groups and store them into a list
        # we also get rid of any lines that we dont need
        groups = [group.split('\n') for group in groups]
        groups = [[line for line in group if line not in ('', 'BEGIN:VEVENT')] for group in groups]
        infile.close()
        return groups
    except FileNotFoundError:
        print ("The file can't be opened (maybe doesn't exist)")
    except PermissionError:
        print ("Lacking permission")    


def print_event(events, start, end):
    x = list(events.keys())
    x.sort()
    x = [key for key in x if (cmdl_object(start) <= key <= cmdl_object(end))]
    count = 0;
    for key in x[0:]:
        if(count > 0):
            print()
        date = '{0} {1:0>2}, {2} ({3})'.format(month(key), key.day, key.year, day_of(key))
        print(date)
        print('-' * len(date))
        # Prints formatted time summary and location under the dashed (every 2 curly == 1 curly printed)
        # event[0] start time, event[1] end time, event[2] summary, event[3] location; hours helper to format time
        for event in events[key]:
            print('{0} to {1}: {2} {{{{{3}}}}}'.format(hours(event[0]), hours(event[1]), event[2], event[3]))
        count += 1


def main():
    p = argparse.ArgumentParser() 
    p.add_argument('--start', type = str, help = 'start date')
    p.add_argument('--end',   type = str, help = 'end date')
    p.add_argument('--file',  type = str, help = 'file being processed')
    args = p.parse_args()

    if not args.file:
        print("Need --file=<ics filename>")
    if not args.start:
        print("Need --start=dd/mm/yyyy")
    if not args.end:
        print("Need --end=dd/mm/yyyy")

    given  = read(args.file)
    events = store(given)
    print_event(events, args.start, args.end)

if __name__ == "__main__":
    main()







