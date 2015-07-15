# Author: Kamil Agrawala
# Email: agrawalk@onid.orst.edu\
# Class: Cs344-001
# Assingment no 2
# Programming Projects program3.py file
# USING ONE LATE GRACE DAY ! (Emailed Professor Prior to deadline as per requirements)

import subprocess

p = subprocess.Popen('who', shell=True,
                     stdout=subprocess.PIPE,
                     stderr=subprocess.STDOUT)
print p.communicate()[0]
