pthread_deadline
================

pthread_deadline test

compilation

  make

usage
  
  sudo ./pthread_deadline <period>:<exec>
  
  ps: exec should be smaller than period
  
sample
  sudo ./pthread_deadline 100000:30000
  
  
ps: Linux kernel version should be 3.14 or newer
