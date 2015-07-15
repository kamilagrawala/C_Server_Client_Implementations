# Author: Kamil Agrawala
# Email: agrawalk@onid.orst.edu
# Class: Cs344-001
# Assingment #1
# Programming Projects problem6.py file

import os;

directory1 = "./lecture_notes";
directory2 = "./examples";
directory3 = "homework";

if not os.path.exists(directory1):
    os.makedirs(directory1);
else: 
    print("Directory ["+ directory1 +"] exsists!\n");

if not os.path.exists(directory2):
    os.makedirs(directory2);
else:
    print("Directory ["+ directory2 +"] exsists!\n");

if not os.path.exists(directory3):
    os.makedirs(directory3);
else:
    print("Directory ["+ directory3 +"] exsists!\n");
