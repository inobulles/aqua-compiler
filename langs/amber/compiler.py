class amber_compiler:
	class token:
		UNKNOWN = 0;
		EXPRESSION = 1;

		WORD = 2;
		OPERATOR = 3;

		STRING = 4;
		NUMBER = 5;

		END_LINE = 6;

		OPEN_PARENTHESES = 7;
		CLOSE_PARENTHESES = 8;

		OPEN_BRACES = 9;
		CLOSE_BRACES = 10;
		
		FUNCTION_CALL = 11
		END_ARGUMENT = 12
		STATEMENT = 13
		BLOCK = 14
		TYPE = 15
		
		type = UNKNOWN
		content = ""
		tokens = []
		stack_pointer = -1
		label_pointer = ""
		call_token = None
		
		def __init__(self, type = UNKNOWN):
			self.type = type
			self.tokens = []
		
		def reference(self):
			if self.stack_pointer >= 0: return "[rbp-%d]" % self.stack_pointer
			if self.label_pointer >= 0: return self.label_pointer
		
		def copy_reference(self, source):
			self.stack_pointer = source.stack_pointer
			self.label_pointer = source.label_pointer
	
	class variable:
		UNKNOWN = 0
		FUNCTION = 1
		
		INT = 2
		UINT = 3
		
		type = UNKNOWN
		name = ""
		
		stack_pointer = -1
		label_pointer = ""
		
		def __init__(self, name = "", stack_pointer = -1, type = UNKNOWN):
			self.name = name
			self.type = type
			self.stack_pointer = stack_pointer
	
	def get_type_from_token(self, string):
		if string == "ftype": return self.variable.FUNCTION
		elif string == "int": return self.variable.INT
		elif string == "uint": return self.variable.UINT
		else: return self.variable.UNKNOWN
	
	FUNCTION_ENTER_FORMAT = "%s: ; WTF\n"
	FUNCTION_LEAVE_FORMAT = "ret ; WTF\n"
	ARGUMENT_REGISTERS = ["rdi", "rsi", "rdx", "rcx"]
	
	code = ""
	tokens = []
	tree = token()
	data = []
	text_section = FUNCTION_ENTER_FORMAT % "main" + "mov rbp rsp ; WTF\nsub rbp 1024 ; WTF\n"
	data_section = ""
	stack_pointer = 0
	statement_count = 0
	function_count = 0
	variables = []
	
	def __init__(self, code):
		self.code = code
	
	def lex(self, code):
		parsing_string = False
		self.tokens.append(self.token())
		
		for current in code:
			white = False
			add_current = False
			
			if parsing_string:
				if current == '"':
					parsing_string = False
					white = True
				else: add_current = True
			elif current in " \n\t;,{}()":
				white = True
				print(current)
			elif current in "+-*/%=<>!.^&~|?":
				white = True
				self.tokens.append(self.token())
				self.tokens[-1].type = self.token.OPERATOR
				add_current = True
			elif ord(current) >= ord('0') and ord(current) <= ord('9'):
				self.tokens[-1].type = self.token.NUMBER
				add_current = True
			elif current == '"':
				self.tokens.append(self.token())
				parsing_string = True
				self.tokens[-1].type = self.token.STRING
			else:
				add_current = True
			
			if add_current:
				self.tokens[-1].content = self.tokens[-1].content + current
			if white:
				if self.tokens[-1].type != self.token.UNKNOWN or self.tokens[-1].content:
					if self.tokens[-1].type == self.token.UNKNOWN:
						if   self.tokens[-1].content in ["if", "func", "class", "return"]: self.tokens[-1].type = self.token.STATEMENT
						elif self.tokens[-1].content in ["int", "uint", "ftype"]: self.tokens[-1].type = self.token.TYPE
					
					self.tokens.append(self.token())
				
				white_token = True
				if current == '(':
					if len(self.tokens) >= 2 and not self.tokens[-2].type in [self.token.OPEN_PARENTHESES, self.token.OPERATOR, self.token.STATEMENT]: # function call
						self.tokens[-1].type = self.token.FUNCTION_CALL
					
					else:
						self.tokens[-1].type = self.token.OPEN_PARENTHESES
				
				elif current == ')': self.tokens[-1].type = self.token.CLOSE_PARENTHESES
				
				elif current == ';': self.tokens[-1].type = self.token.END_LINE
				elif current == ',': self.tokens[-1].type = self.token.END_ARGUMENT
				
				elif current == '{': self.tokens[-1].type = self.token.OPEN_BRACES
				elif current == '}': self.tokens[-1].type = self.token.CLOSE_BRACES
				
				else: white_token = False
				if white_token:
					self.tokens.append(self.token())
		
		length = len(self.tokens)
		for i in reversed(range(length)):
			if self.tokens[i].type == self.token.UNKNOWN and not self.tokens[i].content:
				self.tokens.pop(i)
	
	def build_tree(self, tokens):
		self.tree.tokens.append(self.token(self.token.EXPRESSION))
		branch_stack = [self.tree, self.tree.tokens[-1]]
		
		for current in tokens:
			exit = 0
			enter = 0
			block = False
			
			if current.type in [self.token.END_LINE, self.token.END_ARGUMENT]:
				exit = 1
				enter = 1
			elif current.type in [self.token.OPEN_PARENTHESES, self.token.FUNCTION_CALL]:
				enter = 2
			elif current.type == self.token.CLOSE_PARENTHESES:
				exit = 2
			elif current.type == self.token.OPEN_BRACES:
				enter = 2
				block = True
			elif current.type == self.token.CLOSE_BRACES:
				exit = 3
				enter = 1
				block = False
			else:
				branch_stack[-1].tokens.append(current)
			
			for i in range(exit):
				branch_stack.pop(-1)
			
 			for i in range(enter):
				branch_stack[-1].tokens.append(self.token(self.token.FUNCTION_CALL if current.type == self.token.FUNCTION_CALL and not i else self.token.BLOCK if block and not i else self.token.EXPRESSION))
				
				if current.type == self.token.FUNCTION_CALL and not i:
					branch_stack[-1].tokens[-1].call_token = branch_stack[-1].tokens[-2]
					branch_stack[-1].tokens.pop(-2)
				
				branch_stack.append(branch_stack[-1].tokens[-1])
	
	def add_data(self, bytes):
		if not bytes in self.data:
			self.data.append(bytes)
		
		return "?_amber_data_%d" % self.data.index(bytes)
	
	def compile_token(self, current, write_code = text_section):
		if current.type == self.token.UNKNOWN:
			variable_found = False
			
			for variable in self.variables:
				if current.content == variable.name:
					variable_found = True
					current.stack_pointer = variable.stack_pointer
					break
			
			if not variable_found:
				current.label_pointer = current.content
		
		elif current.type == self.token.STRING:
			current.label_pointer = self.add_data(current.content)
		
		elif current.type == self.token.FUNCTION_CALL:
			write_code = self.compile_token(current.call_token, write_code)
			
			for argument in current.tokens:
				write_code = self.compile_branch(argument, write_code)
			
			for i in range(len(current.tokens)):
				reference = current.tokens[i].reference()
				
				if reference:
					write_code = write_code + "mov %s %s ; WTF\n" % (self.ARGUMENT_REGISTERS[i], reference)
			
			current.stack_pointer = self.stack_pointer
			write_code = write_code + "call %s ; FIXME this comment is needed for some reason ; WTF\nmov %s rax ; WTF\n" % (current.call_token.reference(), current.reference())
			self.stack_pointer += 8
		
		elif current.type == self.token.BLOCK:
			for peice in current.tokens:
				write_code = self.compile_token(peice, write_code)
		
		elif current.type == self.token.EXPRESSION:
			if len(current.tokens) >= 2 and current.tokens[0].type == self.token.TYPE:
				variable = self.variable(current.tokens[1].content, self.stack_pointer, self.get_type_from_token(current.tokens[0].content))
				self.stack_pointer += 8
				
				if len(current.tokens) >= 3 and current.tokens[2].type == self.token.OPERATOR and current.tokens[2].content == "=":
					extension_token = self.token(self.token.EXPRESSION)
					extension_token.tokens = current.tokens[3: len(current.tokens)]
					
					write_code = self.compile_token(extension_token, write_code)
					write_code = write_code + "mov rcx %s ; WTF\nmov [rbp-%d] rcx ; WTF\n" % (extension_token.reference(), variable.stack_pointer)
				
				else:
					write_code = write_code + "mov [rbp-%d] 0 ; WTF\n" % (variable.stack_pointer)
				
				self.variables.append(variable)
			
			elif len(current.tokens) >= 1 and current.tokens[0].type == self.token.STATEMENT:
				if current.tokens[0].content == "if":
					write_code = self.compile_token(current.tokens[1], write_code)
					
					label = "?_amber_statement_%d" % self.statement_count
					self.statement_count += 1
					write_code = write_code + "cmp %s 0 ; WTF\ncnd 1 ; WTF\ncall %s ; WTF\n" % (current.tokens[1].reference(), label)
					
					code = self.FUNCTION_ENTER_FORMAT % label
					code = self.compile_token(current.tokens[2], code)
					write_code = code + self.FUNCTION_LEAVE_FORMAT + write_code
				
				elif current.tokens[0].content == "func":
					variable = self.variable(current.tokens[1].call_token.content, self.stack_pointer, self.variable.FUNCTION)
					self.variables.append(variable)
					self.stack_pointer += 8
					
					label = "?_amber_function_%d" % self.function_count
					variable.label_pointer = label
					self.function_count += 1
					write_code = write_code + "mov [rbp-%d] %s ; maybe? ; WTF\n" % (variable.stack_pointer, label)
					
					code = self.FUNCTION_ENTER_FORMAT % label
					for i in range(len(current.tokens[1].tokens)):
						variable = self.variable(current.tokens[1].tokens[i].tokens[1].content, self.stack_pointer, self.get_type_from_token(current.tokens[1].tokens[i].tokens[0].content))
						self.variables.append(variable)
						self.stack_pointer += 8
						code = code + "mov [rbp-%d] %s ; WTF\n" % (variable.stack_pointer, self.ARGUMENT_REGISTERS[i])
					
					code = self.compile_token(current.tokens[2], code)
					write_code = code + "mov rax 0 ; WTF\n" + self.FUNCTION_LEAVE_FORMAT + write_code
				
				elif current.tokens[0].content == "class":
					name = current.tokens[1].content
					
					for member in current.tokens[2].tokens[0: -1]:
						print "type = %s, name = %s, set = %d" % (member.tokens[0].content, member.tokens[1].content, len(member.tokens))
				
				elif current.tokens[0].content == "return":
					if len(current.tokens) >= 2:
						extension_token = self.token(self.token.EXPRESSION)
						extension_token.tokens = current.tokens[1: len(current.tokens)]
						write_code = self.compile_token(extension_token, write_code) + "mov rax %s ; WTF\n" % extension_token.reference() + self.FUNCTION_LEAVE_FORMAT
					
					else:
						write_code = write_code + "mov rax 0 ; WTF\n" + self.FUNCTION_LEAVE_FORMAT
			
			elif len(current.tokens) >= 3 and not len(current.tokens) % 1:
				first_operator = current.tokens[1]
				
				if first_operator.type == self.token.OPERATOR:
					for i in range(len(current.tokens) / 2 + 1):
						write_code = self.compile_branch(current.tokens[i * 2], write_code)
					
					current.stack_pointer = self.stack_pointer
					
					def get_operator_token_precedence(operator):
						if operator.content in "*/%": return 3
						elif operator.content in "+-":  return 4
						elif operator.content == "=": return 5
						elif operator.content == "":    return 17
						
						return 16
					
					old_index = -1
					
					for i in range(len(current.tokens) / 2):
						went_left = False
						
						min_precedence = 16
						min_index      = 1
						
						for j in range(len(current.tokens) / 2):
							index = j * 2 + 1
							precedence = get_operator_token_precedence(current.tokens[index])
							
							if precedence < min_precedence:
								min_precedence = precedence
								min_index = index
						
						if min_index < old_index:
							went_left = True
						
						old_index = min_index
						operator = current.tokens[min_index]
						
						if operator.content == "\0":
							continue
						
						instruction = "nop"
						
						if   operator.content == "+": instruction = "add"
						elif operator.content == "*": instruction = "mul"
						elif operator.content == "/": instruction = "div"
						elif operator.content == "%": instruction = "mod"
						elif operator.content == "-": instruction = "sub"
						
						elif operator.content == "=":
							write_code = write_code + "mov rax %s ; WTF\nmov %s rax ; WTF\n" % (current.tokens[min_index + 1].reference(), current.tokens[min_index - 1].reference())
						
						operator.content = "\0"
						
						if instruction != "nop":
							if not i:
								write_code = write_code + "mov rax %s ; WTF\n" % current.tokens[min_index - 1].reference()
							
							if went_left: write_code = write_code + "mov rbx rax ; WTF\nmov rax %s ; WTF\n%s rax rbx ; WTF\n" % (current.tokens[min_index - 1].reference(), instruction)
							else:         write_code = write_code + "%s rax %s ; WTF\n" % (instruction, current.tokens[min_index + 1].reference())
					
					write_code = write_code + "mov %s rax ; WTF\n" % current.reference()
					self.stack_pointer += 8
			
			elif len(current.tokens) == 1:
				write_code = self.compile_token(current.tokens[0], write_code)
				current.copy_reference(current.tokens[0])
		
		elif current.type == self.token.NUMBER and current.stack_pointer < 0:
			current.stack_pointer = self.stack_pointer
			write_code = write_code + "mov %s %s ; WTF\n" % (current.reference(), current.content)
			self.stack_pointer += 8
		
		return write_code
	
	def compile_branch(self, tree, write_code = text_section):
		if not tree:
			return
		
		write_code = self.compile_token(tree, write_code)
		
		for i in range(len(tree.tokens)):
			current = tree.tokens[i]
			write_code = self.compile_token(current, write_code)
		
		return write_code
	
	def compile_data(self, data):
		for i in range(len(self.data)):
			self.data_section = self.data_section + "?_amber_data_%d: db" % i
			
			for byte in self.data[i]:
				self.data_section = self.data_section + " %xH" % ord(byte)
			
			self.data_section = self.data_section + " 0 ; WTF\n"
	
	def print_tokens(self, tokens, indent = 0):
		for token in tokens:
			print "%s%d\t%s" % (" " * indent, token.type, token.content)
	
	def print_tree(self, tree, indent = 0):
		for token in tree.tokens:
			print "%s%s\t%s" % (" " * indent, "->" if token.type == self.token.EXPRESSION else "-(" if token.type == self.token.FUNCTION_CALL else "{" if token.type == self.token.BLOCK else str(token.type), token.content)
			self.print_tree(token, indent + 1)
	
	def compile(self):
		self.lex(self.code)
		print "\n=== RAW TOKENS ===\n"
		self.print_tokens(self.tokens)
		
		self.build_tree(self.tokens)
		print "\n=== ABSTRACT SYNTAX TREE ===\n"
		self.print_tree(self.tree)
		
		print "\n=== TEXT SECTION ===\n"
		self.text_section = self.compile_branch(self.tree)
		print self.text_section
		
		print "\n=== DATA SECTION ===\n"
		self.compile_data(self.data)
		print self.data_section
		
		return """SECTION .text   align=1 execute                         ; section number 1, code
""" + self.text_section + """SECTION .data   align=1 noexecute                       ; section number 2, data


SECTION .bss    align=1 noexecute                       ; section number 3, bss


SECTION .rodata align=16 noexecute                      ; section number 4, const
""" + self.data_section

f = open("src/main.a")
code = f.read()
f.close()

compiler = amber_compiler(code)
result = compiler.compile()

f = open("main.asm", "wb")
f.write(result)
f.close()
