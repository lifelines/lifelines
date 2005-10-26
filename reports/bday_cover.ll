/*
 * @progname    bday_cover.ll
 * @version     1 of 1994-11-02
 * @author      Andrew Deacon
 * @category
 * @output      Text
 * @description

A novelty report that lists on which days of the year people were born
and how many people share the same birthday. All valid birthdays
are considered. A valid birthday is one where the extracted birthday,
performed using extractdate(), has a month in the range 1-12 and a day
within that month.

This program works only with LifeLines.

The output is not sorted. The following are examples of
how to sort the output using UNIX sort:
# sort by frequency
sort +2n +0M bday.out
sort +2nr +0M bday.out
# sort by month
sort -M bday.out

*/

        global(julian)
        global(daysinmonth)

proc main ()
{
        table(day_counts)
        list(day_list)

        /* Formats/modes for date functions */
        monthformat(3) dayformat(1) dateformat(1)
        set(julian, 0) /* change to use Julian dates */

        /* Initialize counters */
        set(totaldays, 0) set(totalmonths, 0) set(totalbirths, 0)

        /* Iterate over whole database */
        forindi (indi, num) {

           /* if birthday recorded for individual */
           if (bth, birth(indi)) {

              /* Extract birthday for individual */
              extractdate(bth, birthday, birthmonth, birthyear)
              call get_days_in_month(birthday, birthmonth, birthyear)

              /* if valid birthday */
              if (and(gt(birthday, 0), le(birthday, daysinmonth))) {

                 /* Extract the month name and day */
                 set(bday, concat(substring(stddate(birth(indi)),1,6)," "))

                 /* if existing birthday found - just increment */
                 if(nmatch, lookup(day_counts, bday)) {
                    set(nmatch, add(nmatch, 1))
                 }
                 /* else new birthday - insert */
                 else {
                    set(totaldays, add(totaldays,1))
                    enqueue(day_list, bday)
                    set(nmatch, 1)
                 }
                 insert(day_counts, bday, nmatch)
                 set(totalbirths, add(totalbirths,1))

                 /* Extract the month name */
                 set(bmon, concat(substring(stddate(birth(indi)),1,4),"**"))

                 /* if existing birth month found - just increment */
                 if(nmatch, lookup(day_counts, bmon)) {
                     set(nmatch, add(nmatch, 1))
                 }
                 /* else new birth month - insert */
                 else {
                    set(totalmonths, add(totalmonths,1))
                    enqueue(day_list, bmon)
                    set(nmatch, 1)
                 }
                 insert(day_counts, bmon, nmatch)
              }
           }
        }

        /* Write report to file - use Unix sort to sort output! */
        "Distribution of birth days\n\n"
        "Month & day       Frequency\n\n"
        forlist(day_list, bday, num) {
                bday
                set(nmatch, lookup(day_counts, bday))
                col(sub(25, strlen(d(nmatch))))
                d(nmatch) "\n"
        }
        "Total birthdays in database: " d(totalbirths) "\n"
        "Total days (out of 366)    : " d(totaldays)   "\n"
        "Total months (out of 12)   : " d(totalmonths) "\n"
}

proc get_days_in_month(birthday, birthmonth, birthyear)
{
        /* code from a routine in "dates" by Jim Eggert */
        /* procedure sets global variable daysinmonth */
        set(daysinmonth, 31)
        if (or(le(birthmonth, 0), gt(birthmonth, 12)))
           { set(daysinmonth, 0) }
        elsif (or(or(eq(birthmonth, 9), eq(birthmonth, 4)),
               or(eq(birthmonth, 6), eq(birthmonth, 11))))
           { set(daysinmonth, 30) }
        elsif (eq(birthmonth, 2)) {
               if (and(eq(mod(birthyear, 4), 0),
                   or(julian, or(ne(mod(birthyear, 100), 0),
                                eq(mod(birthyear, 400), 0)))))
                 { set(daysinmonth, 29) }
               else
                 { set(daysinmonth, 28) }
        }
        else
           { set(daysinmonth, 31) }
}

/*

Sample output:

sorted by sort +2nr +0M sample.output and then edited

Distribution of birthdays

Total birthdays in database: 374
Total days (out of 366)    : 236
Total months (out of 12)   : 12

Month & day       Frequency

AUG 12                 6
SEP 12                 5
FEB 10                 4
MAR 03                 4
APR 12                 4
JUN 17                 4
JAN 06                 3
JAN 18                 3
JAN 29                 3
........
*/

/*
Below is a simple C program hack to check if your values
are similar to those generated randomly by the program.
Extract the program from these comment to compile and execute.
Change the RSEED to do different tests; change the ITERATIONS
to vary accuracy. Can also change the NUM_DAYS_REQUIRED
to the value obtained for your database and check if the people
required is similar.

#define RSEED 1576
#define NUM_DAYS 365
#define NUM_DAYS_REQUIRED NUM_DAYS
#define ITERATIONS 2000

#define FALSE 0
#define TRUE 1

static int days[NUM_DAYS];
static int num_got;
static int running_total;


int get_day() {
  return rand() % NUM_DAYS;
}

do_it() {
  int i;
  int j;
  int r;

  for (i = 0; i < NUM_DAYS; i++) {
    days[i] = FALSE;
  }
  num_got = 0;
  i = 0;

  while (num_got < NUM_DAYS_REQUIRED) {
    i++;
    r = get_day();
    if (!days[r]) {
      days[r] = TRUE;
      num_got++;
    }
  }
  printf("Required %d people to cover %d days.\n",i, NUM_DAYS_REQUIRED);
  running_total = running_total + i;
}

main() {
  int i;

  running_total = 0;
  srand(RSEED);
  for (i = 0; i < ITERATIONS; i++) {
    do_it();
  }
  printf("Average was %d.\n",(int)(running_total/ITERATIONS));
}

*/
