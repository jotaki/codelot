#! /usr/bin/python

import socket, select, string

class SocketException(Exception):
	pass

class ClientSocketException(SocketException):
	pass

class ServerSocketException(SocketException):
	pass

class SocketIO:
	def __init__(self, sock=None,type=None):
		self.type = type
		self.socket = sock

	def send(self, data):
		if self.socket is None:
			raise SocketException, "Not connected"

		try:
			self.socket.send(data)
		except socket.error, errstr:
				self.close()

	def read(self):
		if self.socket is None:
			raise SocketException, "Not connected"

		try:
			data = self.socket.recv(2 ** 15 - 1)
		except socket.error, errstr:
			self.close()
			return None

		if data == '':
			self.close()
			return None

		return data

	def fileno(self):
		try:
			return self.socket.fileno()
		except socket.error:
			return -1 # ...

	def close(self):
		self.socket.close()

	def get_type(self):
		return self.type

class ClientSocket(SocketIO):
	def __init__(self, server=None, port=None):
		self.type = 1
		self.connected = False
		self.socket = None

		if server != None and port != None:
			self.connect(server, port)

	def connect(self, server, port):
		if self.connected:
			self.close()

		self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		try:
			self.socket.connect((server, port))
		except socket.error, errstr:
			self.socket.close()
			self.socket = None
			self.connected = False
			raise ClientSocketException, "Could not connect to %s" % errstr

		self.connected = True
		return self

	def close(self):
		if not self.connected:
			return

		self.connected = False
		try:
			self.socket.close()
		except socket.error, errstr:
			pass

class ServerSocket(SocketIO):
	def __init__(self, port=None):
		self.type = 2
		self.socket = None

		if port is not None:
			self.listen(port)

	def listen(self, port):
		self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		self.socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
		try:
			self.socket.bind(('', port))
			self.socket.listen(3)
		except socket.error, errstr:
			raise ServerSocketException, "Couldn't bind: %s" % errstr

	def accept(self):
		try:
			newsock = self.socket.accept()[0]
		except socket.error, errstr:
			raise ServerSocketException, "Couldn't accept: %s" % errstr

		return SocketIO(newsock, 3)

	def close(self):
		if self.socket is None:
			return

		try:
			self.socket.close()
		except socket.error, errstr:
			pass
