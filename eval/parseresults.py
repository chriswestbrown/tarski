#! /usr/bin/python3

###############################################################################
#
# extracts all graph strings from the results files created by Hokulea and prints
# them to a new file in the format (comma seperated) expected by mpievaltarski.
# Takes the reults file as a command line argument.
#
###############################################################################


import sys
import re

file = open(sys.argv[1],"r")
text = file.read()
file.close()
rounds=re.findall("graph string:\\n.*\\n",text)
rounds = [i.lstrip("graph string:\n").rstrip("\n") for i in rounds]
out = open("graph_strings_"+sys.argv[1],"w")
for r in rounds:
    out.write(str(r)+",")
out.flush()
out.close()
