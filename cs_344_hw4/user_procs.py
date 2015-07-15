# Author Kamil Agrawala
# Class: CS344-001       
# Homework Assingment #4 
# Email: agrawalk@onid.orst.edu
# Due: 03/02/15 (with extra credit)                                           
# Program: user_procs.py
# Citations: https://docs.python.org/2/library/getopt.html
#            http://stackoverflow.com/questions/2804543/read-subprocess-stdout-line-by-line

# imports
import getopt, sys, os, subprocess


def Toption():
   import subprocess
   proc = subprocess.Popen('ps -U $USER',shell=True,stdout=subprocess.PIPE)
   while True:
      line = proc.stdout.readline()
      if line != '':
      #the real code does filtering here
         if((line.split()[0]).isdigit()):
            print("\nProccess ID: " + line.split()[0].rstrip() + " Command Name: " + line.split()[3] )
            print("           Thread Count: ")
            proc2 = subprocess.Popen('ps huH p' + line.split()[0] + '| wc -l',shell=True,stdout=subprocess.PIPE)
            print("                   " + proc2.stdout.read().rstrip());
      else:
         break
         
         
def main():

   #print 'ARGV      :', sys.argv[1:]

   options, remainder = getopt.gnu_getopt(sys.argv[1:], 'ahSTZu:U:', ['all=', 'help=', 'Summary=', 'ThreadCount=','Zombies','uid','Username'])

   for opt,arg in options:
      if opt in ('-h', '--help'):
         print("Usage: python user_procs.py <option(hSTZu<uid>U<username>>\n")
         sys.exit(2)
      elif opt in ('-a', '--all'):
         out = subprocess.Popen('ps -U $USER', shell=True, stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
         print out.stdout.read()
      elif opt in ('-S', '--Summary'):
         print("Number of proccesses\n");
         out = subprocess.Popen('ps -u $USER | wc -l', shell=True, stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
         print out.stdout.read()
      elif opt in ('-T', '--ThreadCount'):
         Toption();
      elif opt in ('-Z', '--Zombies'):
         print("Zombies\n");
         out = subprocess.Popen('ps axo user,pid,ppid,command,s | grep -w Z | grep $USER | grep -v \"grep\" | grep -v \"user_procs.py\" ', shell=True, stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
         print out.stdout.read()
      elif opt in ('-u', '--uid'):
         out = subprocess.Popen('ps -U '+arg, shell=True, stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
         print out.stdout.read()
      elif opt in ('-U', '--Username'):
         out  = subprocess.Popen('ps -U '+arg, shell=True, stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
         print out.stdout.read()
      else:
         print("Usage: python user_procs.py <option(hSTZu<uid>U<username>>\n")
         sys.exit(2)
          
if __name__ == "__main__":
    main()

#ps huH p <PID> | wc -l
# ps aux | grep -w Z | grep agrwalk 
