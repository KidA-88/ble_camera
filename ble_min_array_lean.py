import os
import sys
import time
import logging
import sqlite3
import pexpect
import argparse
import configparser
import time
from datetime import datetime

gt = pexpect.spawn('gatttool -I')
gt.expect(r"\[LE\]>")
gt.sendline('connect C0:4B:39:C9:B1:04')
gt.expect(["Connection successful.", r"\[CON\]"], timeout=300)
print("Connected")
while 1:
	print (datetime.now())
	gt.sendline('char-read-hnd c')
	print (datetime.now())
	gt.expect(r"Characteristic value/descriptor: (.+)\r")
	print (datetime.now())
	m = gt.match.group(1).decode("utf8").strip()
	d="".join([chr(int(e, 16)) for e in m.split()])
	single=d.split(' ')
	#print (d.split(' '))
	for x in single:
		print(format(int(x), 'x'))


