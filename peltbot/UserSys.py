#! /usr/bin/python

import string;

class UserSystem:
	def __init__(self, head):
		self.head = head;
		self.passwd = "";
		self.users = [];
		self.auto = [];

	def is_user(self, nick):
		tmp = string.join(self.users, ' ');
		if tmp.find(nick.lower()) != -1:
			return True;

		return False;

	def add(self, nick, passwd):
		if passwd == self.passwd:
			self.add_("!@%", nick);
			return "You are now logged in.";

		return "Invalid password";

	def add_(self, nick, add):
		nick = nick.lower();
		add = add.lower();

		if self.find(nick) and not self.is_user(add):
			self.users.append(add);

	def find(self, nick):
		if nick == "!@%":
			return True;

		u = string.join(self.users, ' ');
		a = string.join(self.auto, ' ');
		if u.find(nick) != -1 or a.find(nick) != -1:
			return True;

		return False;

	def pop(self, nicks):
		nicks = nicks.split(' ');
		for nick in nicks:
			try:
				self.users.remove(nick.lower());
			except ValueError:
				pass;

	def get(self):
		return string.join(self.users, ' ');

	def config(self):
		self.passwd = self.head.get_varkeep().get("botpass");
		self.auto = self.head.get_varkeep().get("botusers").split(' ');
