#! /usr/bin/python

from EZSock import ClientSocket;
from UserSys import UserSystem;
import ForFun, Reminder, pbHandler;

import string, tarfile, imp, sys;

class AliasEngine:
	def __init__(self, pv, pf, sock=None):
		self.sock = sock;
		self.parse_vars = pv;
		self.parse_func = pf;
		self.aliases = {};
		self.imp_a = {};
		self.evals = {};
		self.varkeep = ForFun.VarKeeper("Initial Value");
		self.handler = pbHandler.HandlerEngine(self, sock);
		self.config = {};
		self.user_sys = UserSystem(self);
		self.last_words = [ 'a!b@c', 'PRIVMSG', 'ignored', 'func()' ];

	def get_varkeep(self):
		return self.varkeep;

	def give_words(self, words):
		self.last_words = words;

	def get_words(self):
		return self.last_words;

	def update_config(self, config):
		self.config = config;
		self.handler.update_config(config);

	def set_sock(self, sock):
		self.sock = sock;
		self.handler.set_sock(sock);

	def flush(self):
		self.evals = {};
		self.aliases = {};
		self.imp_a = {};

	def parse_script(self, script):
		for line in script:
			if line == '' or line[0] == ':':
				continue;

			if line[-1] == ';': line = line[:-1];

			(cmd, argv) = self.parse_func(line);
			argc = len(argv);
			if cmd == 'eval':
				eval(argv[0]);
			elif cmd == 'admalias':
				if argv[1].startswith("eval"):
					self.evals[argv[0]] = argv[1][5:-1];
					argv[1:] = argv[2:];

				self.addimportant(argv[0], argv[1:]);
			elif cmd == 'addalias':
				if argv[1].startswith("eval"):
					self.evals[argv[0]] = argv[1][5:-1];
					argv[1:] = argv[2:];

				self.add(argv[0], argv[1:]);
			elif cmd == 'delalias':
				for arg in argv:
					self.remove(arg);
			elif cmd == 'addhandler':
				self.handler.add(argv[0], argv[1:]);
			elif cmd == 'delhandler':
				for arg in argv:
					self.handler.remove(arg);
			elif cmd == 'hook':
				for arg in argv:
					self.handler.addv(arg);
			elif cmd == 'unhook':
				for arg in argv:
					self.handler.removev(arg);
			elif cmd == 'delctcp':
				for arg in argv:
					self.handler.removectcp(arg);
			elif cmd == 'addctcp':
				self.handler.addctcp(argv[0], argv[1:]);
			elif cmd == 'ifnot':
				if self.varkeep.get(argv[0]) == '_INVALID_':
					if argv[1] == 'exit':
						return;

					self.parse_script(argv[1:]);
			elif cmd == 'if':
				if self.varkeep.get(argv[0]) != '_INVALID_':
					if argv[1] == 'exit':
						return;

					self.parse_script(argv[1:]);
			elif cmd == 'exit':
				return;
			elif cmd == 'script':
				try:
					scr = open(argv[0], 'r');
					_script = scr.read().split(':unload=')[0];
					self.parse_script(_script.split('\n'));
					scr.close();
				except IOError:
					pass;
			elif cmd == 'import': # import a tar file.
				try:
					tf = tarfile.open(argv[0], 'r');
					for f in tf:
						if not f.isreg():
							continue;

						scr = tf.extractfile(f.name);
						_script = scr.read().split(':unload=')[0];
						self.parse_script(_script.split('\n'));
						scr.close();
					tf.close();
				except IOError:
					pass;
			elif cmd == 'flush':
				self.flush();
			else:
				self.parse(True, self.last_words, cmd, argv);

	def add(self, cmd, do_what):
		self.aliases = self.newcmd(self.aliases, cmd, do_what);

	def newcmd(self, dict, cmd, do_what):
		idx = 1;

		for dw in do_what:
			if dw[0] == '~':
				_dw = dw.split(' ');
				_dw[0] = _dw[0][1:];

				if _dw[0] == 'wait': # we wait for a response, to send the rest.
					self.handler.add_tmp(_dw[1], do_what[idx+1:]);
					break;

				try:
					tmp = str(dict[cmd]) + "\r\n" + str(dict[_dw[0]][0]);
					dict[cmd] = [tmp];
				except KeyError:
					try:
						if len(_dw) != 1:
							tmp = dict[_dw[0]][0];
							tmp = tmp.replace('$-', string.join(_dw[1:], ' '));
							dict[cmd] = [tmp];
						else:
							dict[cmd] = dict[_dw[0]];
					except KeyError:
						pass;
			else:
				try:
					tmp = str(dict[cmd][0]) + "\r\n" + dw;
					dict[cmd] = [tmp];
				except KeyError:
					try:
						dict[cmd] = [dw];
					except KeyError:
						pass;
		idx += 1;
		
		return dict;

	def addimportant(self, cmd, do_what):
		self.imp_a = self.newcmd(self.imp_a, cmd, do_what);

	def list(self):
		for a in self.aliases:
			print a, '=', self.aliases[a];

	def listimp(self):
		for a in self.imp_a:
			print a, '=', self.imp_a[a];

	def remove(self, cmd):
		try:
			self.aliases.pop(cmd);
		except KeyError:
			pass;

		try:
			self.imp_a.pop(cmd);
		except KeyError:
			pass;

		try:
			self.evals.pop(cmd);
		except KeyError:
			pass;

	def parse(self, is_user, words, cmd, _argv):
		try:
			if is_user:
				argv = self.parse_vars(_argv, words);
				do_what = self.parse_a(cmd, self.imp_a[cmd], argv, words);
				do_what = self.parse_vars(do_what, words)[0];
				if do_what != '_NOTHING_':
					self.sock.send('%s\r\n' % do_what);
			else:
				raise KeyError;
		except KeyError:
			try:
				argv = self.parse_vars(_argv, words);
				do_what = self.parse_a(cmd, self.aliases[cmd], argv, words);
				do_what = self.parse_vars(do_what, words)[0];
				if do_what != '_NOTHING_':
					self.sock.send('%s\r\n' % do_what);
			except KeyError:
				pass;

	def parse_a(self, cmd, action, argv, words = []):
		action = action[0];
		idx = 1;
		try:
			_eval = self.evals[cmd];
		except KeyError:
			_eval = "";

		try:
			action = action.replace('$0', cmd);
			_eval = _eval.replace('$0', cmd);

			action = action.replace('$-', string.join(argv, ' '));
			_eval = _eval.replace('$-', string.join(argv, ' '));

			if len(argv) > 0:
				action = action.replace('${}-', string.join(argv, ' '));
				_eval = _eval.replace('${}-', string.join(argv, ' '));

				action = action.replace('${}', argv[0]);
				_eval = _eval.replace('${}', argv[0]);
			else:
				action = action.replace('${}-', '$N');
				_eval = _eval.replace('${}-', '$N');
				action = action.replace('${}', '$N');
				_eval = _eval.replace('${}', '$N');

			for arg in argv:

				action = action.replace('$' + str(idx) + '-', string.join(argv[idx-1:], ' '));
				_eval = _eval.replace('$' + str(idx) + '-', string.join(argv[idx-1:], ' '));

				action = action.replace('$' + str(idx), arg);
				_eval = _eval.replace('$' + str(idx), arg);

				idx += 1;
		except:
			raise;

		while action.find('$^') != -1:
			try:
				key = action.split('$^')[1];
				if key.find('^') == -1:
					break;

				key = key.split('^')[0];
				action = action.replace('$^' + key + '^', self.varkeep.get(key));
			except:
				pass;

		while _eval.find('$^') != -1:
			try:
				key = _eval.split('$^')[1];
				if key.find('^') == -1:
					break;

				key = key.split('^')[0];
				_eval = _eval.replace('$^' + key + '^', self.varkeep.get(key));
			except:
				pass;

		if _eval != "":
			if _eval.find('$') != -1:
				_eval= self.parse_vars([_eval], words)[0];

			try:
				sock = self.sock;
				x = str(eval(_eval));
			except:
				x = "Error";

			action = action.replace('$eval', x);

		return [action];
