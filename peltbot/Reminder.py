#! /usr/bin/python

from EZSock import ClientSocket;
import threading;

class Reminder:
	def __init__(self, sock, sendto, nick, timer, remind):
		self.sock = sock;
		self.sendto = sendto;
		self.nick = nick;
		self.remind = remind;
		t = self.parse_timer(timer);
		if t != 0:
			self.timer = threading.Timer(t, self.func);
			self.timer.start();

	def func(self):
		self.sock.send('PRIVMSG %s :%s: %s\r\n' % (self.sendto, self.nick, self.remind));

	def parse_timer(self, hms_str):
		sec = 0.0;
		hms = hms_str.split(':');
		try:
			a = int(hms[0]);
			b = int(hms[1]);
			c = int(hms[2]);
		except ValueError:
			return 0.0;
		except IndexError:
			pass;

		if len(hms) == 1:
			sec = a;
		elif len(hms) == 2:
			sec = b;
			sec += a * 60;
		elif len(hms) == 3:
			sec = c
			sec += b * 60;
			sec += a * 3600;

		return float(sec);
