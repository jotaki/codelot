#! /usr/bin/python
"""
	This is just stuff for the peltbot, that is more for fun things, than
	anything else.
"""

from EZSock import ClientSocket;
import aspell, string, select, sys, imp, os;

def pyscript(script):
	(fp, path, descr) = imp.find_module(script, os.listdir("."));

	try:
		return imp.load_module(script, fp, path, descr);
	finally:
		if fp:
			fp.close();

def rot13(orig):
	s = '';

	for ch in orig:
		if ord(ch) >= ord('A') and ord(ch) <= ord('Z'):
			n = ((ord(ch) - 52) % 26) + 65;
			s += chr(n);
		elif ord(ch) >= ord('a') and ord(ch) <= ord('z'):
			n = ((ord(ch) - 84) % 26) + 97;
			s += chr(n);
		else:
			s += ch;

	print s;
	return s;

def md5sum(s):
	from md5 import new;
	return new(s).hexdigest();

def spell_it(word):
	sp_engine = aspell.Speller('lang', 'en');
	if sp_engine.check(word) != 1:
		suggest = string.join(sp_engine.suggest(word), ', ');
		return "Some suggestions for %s are: %s." % (word, suggest);
	else:
		return "The word `%s' appears to be correct" % (word);

def get_weather(zipcode):
	http = ClientSocket('www.weather.com', 80);
	http.send('GET /weather/local/%s HTTP/1.0\r\n' % zipcode);
	http.send('Host: www.weather.com\r\n');
	http.send('Referer: http://www.weather.com/\r\n');
	http.send('Accept: */*\r\n\r\n');

	city = zipcode;

	while True:
		try:
			(iw, ow, ew) = select.select([http], [], [], 1);
		except ValueError:
			break;

		if iw is None:
			break;

		html = str(http.read());

		t_index = html.find('<TITLE>Local Weather Forecast for ');
		if t_index != -1:
			city_i = html.find('(%s)' % zipcode);
			city = html[t_index + 34:city_i];

		temp_i = html.find('<B CLASS=obsTempTextA>');
		temp_i2 = html.find('&deg;F</B>');

		if temp_i != -1 and temp_i2 != -1:
			temp = html[temp_i + 22:temp_i2];
			break;

	http.close();
	return "It is currently %sF in %s" % (temp, city);

class VarKeeper:
	def __init__(self, v = ''):
		self.table = {};

	def set(self, key, val):
		self.table[key] = val;
		return self.get(key);

	def get(self, key):
		try:
			return self.table[key];
		except KeyError:
			return "_INVALID_";

	def pop(self, key):
		try:
			return self.table.pop(key);
		except KeyError:
			return "No Index";
