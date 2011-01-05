#! /usr/bin/python

from EZSock import ClientSocket;
from ForFun import VarKeeper;
import time, string;

class HandlerEngine:
	def __init__(self, head, sock=None):
		self.sock = sock;
		self.handlers = {};
		self.ctcps = {};
		self.config = {};
		self.handv = {};
		self.tmp_hooks = {};
		self.head = head;

	def update_config(self, config):
		self.config = config;

	def set_sock(self, sock):
		self.sock = sock;

	def flush(self):
		self.handlers = {};
		self.ctcps = {};
		self.handv = {};

	def addv(self, msg):
		self.handv[msg] = True;

	def removev(self, msg):
		try:
			self.handv.pop(msg);
		except KeyError:
			pass;

	def add(self, msg, action):
		for act in action:
			try:
				tmp = str(self.handlers[msg][0]) + "\r\n" + act;
				self.handlers[msg] = [tmp];
			except KeyError:
				self.handlers[msg] = [act];

	def add_tmp(self, msg, action):
		self.add(msg, action);
		self.tmp_hooks[msg] = True;

	def addctcp(self, ctcp, action):
			self.ctcps[ctcp] = action;

	def removectcp(self, ctcp):
		try:
			self.ctcps.pop(ctcp);
		except KeyError:
			pass;

	def remove(self, msg):
		try:
			self.handlers.pop(msg);
		except KeyError:
			pass;

	def parse(self, msg, parms):
		try:
			if parms[0][1:parms[0].index('!')] == self.config['nick']:
				return;
		except:
			pass;

		try:
			tmp = self.handv[msg];
			self.head.get_varkeep().set(msg, string.join(parms, ' '));
			self.head.get_varkeep().set(msg + "_0", parms[0]);
			self.head.get_varkeep().set(msg + "_1", parms[1]);
			self.head.get_varkeep().set(msg + "_2", parms[2]);

			tmp = parms[3].split(' ');
			idx = 3;
			for t in tmp:
				self.head.get_varkeep().set(msg + "_" + str(idx), t);
				idx += 1;

		except:
			pass;

		try:
			if msg == 'PRIVMSG' and parms[3][0] == '\1':
				ctcp = parms[3].split(' ')[0][1:];
				if ctcp[-1] == '\1': ctcp = ctcp[:-1];

				action = self.parse_vars(self.ctcps[ctcp], parms);
			else:
				action = self.parse_vars(self.handlers[msg], parms);

			if action == '-':
				self.head.varkeep.pop(action[1:]);
			else:
				self.sock.send("%s\r\n" % action);

			self.tmp_hooks.pop(msg);
			self.remove(msg);
		except KeyError:
			pass;

	def vk_get(self, key):
		try:
			return self.head.get_varkeep().get(key);
		except:
			return "Invalid";

	def parse_vars(self, act, parms):
		act = act[0];

		while act.find('$^') != -1:
			try:
				key = act.split('$^')[1];
				if key.find('^') == -1:
					break;

				key = key.split('^')[0];
				act = act.replace('$^' + key + '^', self.vk_get(key));
			except:
				raise;

		try:
			if parms[0][0] == ':': parms[0] = parms[0][1:];
			if parms[1][0] == ':': parms[1] = parms[1][1:];
			if parms[2][0] == ':': parms[2] = parms[2][1:];
			if parms[3][0] == ':': parms[3] = parms[3][1:];

			act = act.replace('$0', parms[0]);
			act = act.replace('$1', parms[1]);
			act = act.replace('$2', parms[2]);
			act = act.replace('$3', parms[3]);
		except IndexError:
			pass;

		try:
			if parms[0].find('!') != -1:
				nick = parms[0][:parms[0].index('!')];
				user = parms[0][parms[0].index('!')+1:parms[0].index('@')];
				host = parms[0][parms[0].index('@')+1:];
			else:
				nick = user = host = parms[0];

			act = act.replace('$N', nick);
			act = act.replace('$U', user);
			act = act.replace('$H', host);
			act = act.replace('%1', str(chr(1)));
			act = act.replace('$T', time.ctime());
			act = act.replace('$_', self.config['nick']);

			act = self.head.parse_vars([act], self.head.get_words())[0];
		except IndexError:
			pass;
		except KeyError:
			pass;

		return act;
