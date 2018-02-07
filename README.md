# Implementing Linux Pipeline of Commands using System Calls in C

This project is part of the Operating Systems Class at the University of Minnesota CSCI 4061 to fully understand its purpose it is highly recommended to read the Handout.pdf file

## Getting Started

* To run:
  1) run the command make
  2) run the command ./a.out

* To clean up the directory from object and executable files, run the command: make clean

### Important Notes

* The program assumes the following limitations:
   1) can pipe up to 8 commands
   2) maximum command length: 2048 characters
   3) maximum input line length: 2048

* A logfile is created in the same directory with detailed information about the run

* The result/output of the piped commands is printed to the std out
