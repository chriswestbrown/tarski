#!/usr/bin/env python3
from subprocess import Popen, PIPE

p = Popen(["../bin/tarski","-q", "-t", "60", "+N20000000"],stdout=PIPE,stdin=PIPE)
res = p.communicate('(display (+ 3 4) "\\n") (quit)'.encode())[0]
print("Tarski said: " + res.decode());
p.terminate()
