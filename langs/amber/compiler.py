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
			if self.stack_pointer >= 0: return "[rsp-%d]" % self.stack_pointer
			if self.label_pointer >= 0: return self.label_pointer
		
		def copy_reference(self, source):
			self.stack_pointer = source.stack_pointer
			self.label_pointer = source.label_pointer
	
	class variable:
		UNKNOWN = 0
		
		INT = 1
		UINT = 2
		
		type = UNKNOWN
		name = ""
		stack_pointer = -1
		
		def __init__(self, name = "", stack_pointer = -1, type = UNKNOWN):
			self.name = name
			self.type = type
			self.stack_pointer = stack_pointer
	
	code = ""
	tokens = []
	tree = token()
	data = []
	text_section = "main:\n"
	data_section = ""
	stack_pointer = 0
	statement_count = 0
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
			elif current in "+-*/%=<>!.^&~|":
				self.tokens[-1].type = self.token.OPERATOR
				add_current = True
			elif ord(current) >= ord('0') and ord(current) <= ord('9'):
				self.tokens[-1].type = self.token.NUMBER
				add_current = True
			elif current == '"':
				parsing_string = True
				self.tokens[-1].type = self.token.STRING
			else:
				add_current = True
			
			if add_current:
				self.tokens[-1].content = self.tokens[-1].content + current
			elif white:
				if self.tokens[-1].type != self.token.UNKNOWN or self.tokens[-1].content:
					if self.tokens[-1].type == self.token.UNKNOWN:
						if   self.tokens[-1].content in ["if", "elif", "else"]: self.tokens[-1].type = self.token.STATEMENT
						elif self.tokens[-1].content in ["int", "uint"]: self.tokens[-1].type = self.token.TYPE
					
					self.tokens.append(self.token())
				
				if current == '(':
					if len(self.tokens) >= 2 and not self.tokens[-2].type in [self.token.OPERATOR, self.token.STATEMENT]: # function call
						self.tokens[-1].type = self.token.FUNCTION_CALL
					
					else:
						self.tokens[-1].type = self.token.OPEN_PARENTHESES
				
				elif current == ')': self.tokens[-1].type = self.token.CLOSE_PARENTHESES
				
				elif current == ';': self.tokens[-1].type = self.token.END_LINE
				elif current == ',': self.tokens[-1].type = self.token.END_ARGUMENT
				
				elif current == '{': self.tokens[-1].type = self.token.OPEN_BRACES
				elif current == '}': self.tokens[-1].type = self.token.CLOSE_BRACES
	
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
		
		return "?_amber_d%d" % self.data.index(bytes)
	
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
			
			argument_registers = ["rdi", "rsi", "rdx", "rcx"]
			for i in range(len(current.tokens)):
				write_code = write_code + "mov %s %s\n" % (argument_registers[i], current.tokens[i].reference())
			
			current.stack_pointer = self.stack_pointer
			write_code = write_code + "call %s\nmov %s rax\n" % (current.call_token.reference(), current.reference())
			self.stack_pointer += 8
		
		elif current.type == self.token.BLOCK:
			for peice in current.tokens:
				write_code = self.compile_token(peice, write_code)
		
		elif current.type == self.token.EXPRESSION:
			if len(current.tokens) == 1:
				write_code = self.compile_token(current.tokens[0], write_code)
				current.copy_reference(current.tokens[0])
			
			elif len(current.tokens) >= 2 and current.tokens[0].type == self.token.TYPE:
				variable = self.variable(current.tokens[1].content, self.stack_pointer)
				self.stack_pointer += 8
				
				if current.tokens[0].content == "int": variable.type = self.variable.INT
				if current.tokens[0].content == "uint": variable.type = self.variable.UINT
				
				if len(current.tokens) >= 3 and current.tokens[2].type == self.token.OPERATOR and current.tokens[2].content == "=":
					write_code = self.compile_token(current.tokens[3], write_code)
					write_code = write_code + "mov [rsp-%d] %s\n" % (variable.stack_pointer, current.tokens[3].reference())
				
				else:
					write_code = write_code + "mov [rsp-%d] 0\n" % (variable.stack_pointer)
				
				self.variables.append(variable)
			
			elif len(current.tokens) >= 2 and current.tokens[0].type == self.token.STATEMENT:
				has_condition = current.tokens[1].type == self.token.EXPRESSION
				
				if has_condition:
					write_code = self.compile_token(current.tokens[1], write_code)
				
				label = "?_amber_s%d" % self.statement_count
				self.statement_count += 1
				write_code = write_code + "cmp %s 1\ncnd z\ncall %s\n" % (current.tokens[1].reference(), label)
				
				code = "%s:\n" % label
				code = self.compile_token(current.tokens[has_condition + 1], code)
				write_code = code + "ret\n" + write_code
			
			elif len(current.tokens) >= 3 and not len(current.tokens) % 1:
				first_operator = current.tokens[1]
				
				if first_operator.type == self.token.OPERATOR:
					for i in range(len(current.tokens) / 2 + 1):
						write_code = self.compile_branch(current.tokens[i * 2], write_code)
					
					current.stack_pointer = self.stack_pointer
					
					def get_operator_token_precedence(operator):
						if   operator.content in "!=":  return 2
						elif operator.content in "*/%": return 3
						elif operator.content in "+-":  return 4
						
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
						instruction = "nop"
						
						if   operator.content == "+": instruction = "add"
						elif operator.content == "*": instruction = "mul"
						elif operator.content == "/": instruction = "div"
						elif operator.content == "%": instruction = "mod"
						elif operator.content == "-": instruction = "sub"
						
						operator.content = "\0"
						
						if not i:
							write_code = write_code + "mov rax %s\n" % current.tokens[min_index - 1].reference()
						
						if went_left: write_code = write_code + "mov rbx rax\nmov rax %s\n%s rax rbx\n" % (current.tokens[min_index - 1].reference(), instruction)
						else:         write_code = write_code + "%s rax %s\n" % (instruction, current.tokens[min_index + 1].reference())
					
					write_code = write_code + "mov %s rax\n" % current.reference()
					self.stack_pointer += 8
		
		elif current.type == self.token.NUMBER and current.stack_pointer < 0:
			current.stack_pointer = self.stack_pointer
			write_code = write_code + "mov %s %s\n" % (current.reference(), current.content)
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
			self.data_section = self.data_section + "?_amber_d%d: db" % i
			
			for byte in self.data[i]:
				self.data_section = self.data_section + " %xH" % ord(byte)
			
			self.data_section = self.data_section + " 0\n"
	
	def print_tokens(self, tokens, indent = 0):
		for token in tokens:
			print "%s%d\t%s" % (" " * indent, token.type, token.content)
	
	def print_tree(self, tree, indent = 0):
		for token in tree.tokens:
			print "%s%s\t%s" % (" " * indent, "->" if token.type == self.token.EXPRESSION else "-(" if token.type == self.token.FUNCTION_CALL else "{" if token.type == self.token.BLOCK else str(token.type), token.content)
			self.print_tree(token, indent + 1)
	
	def compile(self):
		self.lex(self.code)
		#~ print "\n=== RAW TOKENS ===\n"
		#~ self.print_tokens(self.tokens)
		
		self.build_tree(self.tokens)
		print "\n=== ABSTRACT SYNTAX TREE ===\n"
		self.print_tree(self.tree)
		
		print "\n=== TEXT SECTION ===\n"
		self.text_section = self.compile_branch(self.tree)
		print self.text_section
		
		print "\n=== DATA SECTION ===\n"
		self.compile_data(self.data)
		print self.data_section
		
		return "section text align=1 execute\n" + self.text_section + "section data align=1\n" + self.data_section

f = open("src/main.a")
code = f.read()
f.close()

compiler = amber_compiler(code)
result = compiler.compile()

f = open("main.asm", "wb")
f.write(result)
f.close()
