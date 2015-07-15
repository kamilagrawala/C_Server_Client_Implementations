# Author: Kamil Agrawala
# Email: agrawalk@onid.orst.edu
# Class: Cs344-001
# Assingment #1
# Programming Projects problem7.py file



import getopt, sys, string,os

Term = ''
Class = ''
link = '/usr/local/classes/eecs/<term>/<class>/README'
link2 = '/usr/local/classes/eecs/<term>/<class>/src'

def main():

    print 'ARGV      :', sys.argv[1:]

    options, remainder = getopt.gnu_getopt(sys.argv[1:], 't:c:', ['term=', 'class='])

    #print 'OPTIONS   :', options

    for opt,arg in options:
        if opt in ('-t', '--term'):
            Term = arg
        if opt in ('-c', '--class'):
            Class = arg

    new_link = string.replace(link, '<class>', Class)
    new_link = string.replace(new_link, '<term>', Term)
    new_link2 = string.replace(link2, '<class>', Class)
    new_link2 = string.replace(new_link2, '<term>', Term)

    print("link = [" + new_link + "]\n");
    print("link2 = [" + new_link2 + "]\n");

    os.symlink(new_link,'./README');
    os.symlink(new_link2,'./src_class');
    
if __name__ == "__main__":
    main()
