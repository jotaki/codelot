#! /usr/bin/python

from EZSock import ClientSocket;
import select, string, os, sys, Reminder, ForFun, time, pbAlias, pbHandler;

DEBUG=True;

if not DEBUG:
	pid = os.fork();
	if pid:
		sys.exit(0);

config = { "server":"", "port":"", \
	"nick":"", "usern":"", \
	"useri":"", "join":"", \
	"topic":{}, "_start":time.strftime('%s') };

def bot_uptime():
	global config;
	_start = int(config['_start']);
	current = int(time.strftime('%s')) - _start;
	uptime = "%dd %dh %dm %ds" % (current/3600/24, current/3600%24, current/60%60, current%60);
	return uptime;

def parse_vars(args, words):
	global config, DEBUG;
	newargs = [];
	f_nick = f_user = f_host = '';

	try:
		f_nick = words[0][1:words[0].index('!')];
		f_user = words[0][words[0].index('!')+1:words[0].index('@')];
		f_host = words[0][words[0].index('@')+1:];
	except:
		pass;

	for arg in args:
		arg = arg.replace('$_', config['nick']);
		if words[2][0] == '#':
			arg = arg.replace('$C', words[2]);
		else:
			arg = arg.replace('$C', f_nick);

		arg = arg.replace('$V', '1.00');
		arg = arg.replace('$DB', str(DEBUG));
		arg = arg.replace('%1', str(chr(1)));
		arg = arg.replace('$T', time.ctime());
		arg = arg.replace('$N', f_nick);
		arg = arg.replace('$U', f_user);
		arg = arg.replace('$H', f_host); arg = arg.replace('$uptime', bot_uptime());
		try:
			arg = arg.replace('$topic', config['topic'][words[2]]);
		except KeyError:
			arg = arg.replace('$topic', 'Not a channel??');

		arg = arg.replace('$$', '$');
		newargs.append(arg);

	return newargs;

def parse_func(func):
	cmd = '';
	args = [];

	if func[-1] != ')':
		return [cmd, args];

	try:
		iq = False;
		escaped = False;
		tmp = func.split('(');
		cmd = tmp[0];
		argss = func[len(cmd) + 1:-1];
		tmp = '';

		for ch in argss:
			if ch == '\"' and escaped == False:
				iq = not iq;
				ch = '';
			elif ch == ',' and iq == False:
				args.append(tmp.strip());
				ch = tmp = '';
			elif ch == '\\':
				if escaped == False:
					escaped = True;
					ch = '';

			tmp += ch;
			if ch != '' and escaped == True:
				escaped = False;

		if iq != False:
			raise;

		if tmp != '':
			args.append(tmp.strip());

	except:
		return [cmd, args];

	return [cmd, args];

def parse_line(sock, line):
	global DEBUG, config, alias_engine;
	words = line.split(' ');

	if DEBUG: print line;
	alias_engine.update_config(config);

	try:
		try:
			tmp = string.join(words[3:], ' ');
			if tmp[0] == ':':
				tmp = tmp[1:]

			words[3] = tmp
			words[4:] = '';
				
		except IndexError:
			pass;
		
		if words[0][0] != ':':
			alias_engine.handler.parse(words[0], words);
		else:
			alias_engine.handler.parse(words[1], words);
	except IndexError:
		pass;

	words = line.split(' ');
	try:
		if words[0] == 'PING':
			sock.send("PONG %s\r\n" % words[1]);
			return;
		elif words[1] == '433' and words[2] == '*':
			config['nick'] = config['nick'] + '_';
			sock.send('NICK %s\r\n' % config['nick']);
			return;
		elif words[1] == 'NICK':
			config['nick'] = words[2][1:];
			return;
		elif words[0] == 'ERROR':
			sock.close();
			return;
		elif words[1] == 'TOPIC':
			config['topic'][words[2]] = string.join(words[3:], ' ')[1:];
			return;
		elif words[1] == '332':
			config['topic'][words[3]] = string.join(words[4:], ' ')[1:];
			return;
		elif words[1] != 'PRIVMSG':
			return;
	except IndexError:
		return;

	alias_engine.give_words(words);

	frm_nick = words[0][1:words[0].index('!')].lower();
	channel = words[2];

	if line.find('(sp?)') != -1:
		for word in words[3:]:
			if word.endswith('(sp?)') and word != '(sp?)':
				if word[0] == ':': word = word[1:];

				sock.send("PRIVMSG %s :%s\r\n" % (channel, ForFun.spell_it(word[0:-5])));
				return;

	if line.find('(') == -1 or line.find(')') == -1:
		return;

	func = string.join(words[3:], ' ')[1:];
	if func[-1] == ';': func = func[:-1];

	(cmd, argv) = parse_func(func);
	cmd = cmd.lower();

	_argv = argv; # for addalias
	argv = parse_vars(argv, words);
	argc = len(argv);
	all_args = string.join(argv, ' ');

	is_user = alias_engine.user_sys.is_user(frm_nick);

	if argc != 0 and is_user: 
		if cmd == 'hook':
			for arg in argv:
				alias_engine.handler.addv(arg);
		elif cmd == 'unhook':
			for arg in argv:
				alias_engine.handler.removev(arg);
		elif cmd == 'addalias' and argc >= 2:
			alias_engine.add(argv[0], _argv[1:]);
		elif cmd == 'admalias' and argc >= 2:
			alias_engine.addimportant(argv[0], _argv[1:]);
		elif cmd == 'delalias':
			alias_engine.remove(argv[0]);
		elif cmd == 'script':
			try:
				scr = open(argv[0], 'r');
				_script = scr.read().split(':unload=')[0];
				scr.close();
			except IOError:
				return;

			script = _script.split('\n');
			alias_engine.parse_script(script);
		elif cmd == 'unload':
			try:
				scr = open(argv[0], 'r');
				_script = scr.read().split(':unload=')[1];
				scr.close();
			except IOError:
				return;
			except IndexError:
				try:
					scr.close();
				except IOError:
					pass;
				return;

			script = _script.split('\n');
			alias_engine.parse_script(script);
	else:
		if cmd == 'flush' and is_user:
			alias_engine.flush();

	alias_engine.parse(is_user, words, cmd, argv);

def strip_r(line):
	t = list(line);
	if line.find('\r') != -1:
		t.pop(line.find('\r'))
		line = string.join(t, '');

	return line;

sock = ClientSocket(config['server'], int(config['port']));

sock.send("NICK %s\r\n" % config['nick']);
sock.send("USER %s * * :%s\r\n" % (config['usern'], config['useri']));

alias_engine = pbAlias.AliasEngine(parse_vars, parse_func, sock);

try:
	scr = open('default.scr', 'r');
	init_a = scr.read().split(':unload=')[0];
	scr.close();
except IOError:
	pass;

script = init_a.split('\n');
alias_engine.parse_script(script);

while True:
	try:
		(iw, ow, ew) = select.select([sock], [], [], 60);
	except ValueError:
		break;

	if iw is None:
		continue;

	data = sock.read();
	lines = data.split('\n');
	for line in lines:
		if line == '':
			continue;

		line = strip_r(line);
		parse_line(sock, line);

