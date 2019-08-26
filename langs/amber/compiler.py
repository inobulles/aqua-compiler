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
		
		def reference(self, encapsulated):
			if self.stack_pointer >= 0: return "cad bp sub %d\t" % self.stack_pointer + encapsulated + "?ad"
			if self.label_pointer >= 0: return encapsulated + self.label_pointer
		
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
		depth = 0
		
		stack_pointer = -1
		label_pointer = ""
		
		def __init__(self, depth = 0, name = "", stack_pointer = -1, type = UNKNOWN):
			self.depth = depth
			self.name = name
			self.type = type
			self.stack_pointer = stack_pointer
	
	def get_type_from_token(self, string):
		if string == "ftype": return self.variable.FUNCTION
		elif string == "int": return self.variable.INT
		elif string == "uint": return self.variable.UINT
		else: return self.variable.UNKNOWN
	
	FUNCTION_ENTER_FORMAT = ":%s:\n"
	FUNCTION_LEAVE_FORMAT = "ret\n"
	
	code = ""
	tokens = []
	tree = token()
	data = []
	text_section = ":main:\tmov bp sp\tsub bp 1024\n"
	data_section = ""
	stack_pointer = 0
	statement_count = 0
	function_count = 0
	inline_count = 0
	variables = []
	depth = 0
	
	def __init__(self, code):
		self.code = code
	
	def lex(self, code): # lexer
		self.tokens.append(self.token())
		
		parsing_operator = False
		parsing_string = False
		
		for current in code:
			white = False
			add_current = False
			was_operator = False
			
			if parsing_string:
				if current == '"':
					parsing_string = False
					white = True
				else: add_current = True
			elif current in " \n\t;,{}()":
				white = True
				print(current)
			elif current in "+-*/%=<>!.^&~|?":
				if not parsing_operator:
					self.tokens.append(self.token())
				
				was_operator = True
				self.tokens[-1].type = self.token.OPERATOR
				add_current = True
			elif not len(self.tokens[-1].content) and ord(current) >= ord('0') and ord(current) <= ord('9'):
				self.tokens[-1].type = self.token.NUMBER
				add_current = True
			elif current == '"':
				self.tokens.append(self.token())
				parsing_string = True
				self.tokens[-1].type = self.token.STRING
			else:
				add_current = True
			
			if parsing_operator and not was_operator:
				self.tokens.append(self.token())
			
			parsing_operator = was_operator
			
			if add_current:
				self.tokens[-1].content = self.tokens[-1].content + current
			if white:
				if self.tokens[-1].type != self.token.UNKNOWN or self.tokens[-1].content:
					if self.tokens[-1].type == self.token.UNKNOWN:
						if   self.tokens[-1].content in ["if", "func", "class", "return", "while", "break"]: self.tokens[-1].type = self.token.STATEMENT
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
	
	def build_tree(self, tokens): # AST generator
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
		
		return "!amber_data_%d" % self.data.index(bytes)
	
	def compile_token(self, current, write_code = text_section): # compile a single token
		if current.type == self.token.UNKNOWN: # variable, function names, or other user-named stuff
			variable_found = False
			
			for variable in self.variables:
				if current.content == variable.name:
					variable_found = True
					current.stack_pointer = variable.stack_pointer
					break
			
			if not variable_found:
				current.label_pointer = current.content
		
		elif current.type == self.token.STRING: # string literals
			current.label_pointer = self.add_data(current.content)
		
		elif current.type == self.token.FUNCTION_CALL: # function calls
			write_code = self.compile_token(current.call_token, write_code)
			
			for argument in current.tokens:
				write_code = self.compile_branch(argument, write_code)
			
			for i in range(len(current.tokens)):
				reference = current.tokens[i].reference("mov a%d " % i)
				
				if reference:
					write_code = write_code + reference + "\n"
			
			current.stack_pointer = self.stack_pointer
			write_code = write_code + current.call_token.reference("cal ") + "\n" + current.reference("mov ") + " g0\n"
			self.stack_pointer += 8
		
		elif current.type == self.token.BLOCK: # blocks
			self.depth += 1
			
			for peice in current.tokens:
				write_code = self.compile_token(peice, write_code)
			
			length = len(self.variables)
			for i in reversed(range(length)):
				if self.variables[i].depth < self.depth: break
				else: self.variables.pop(i)
			
			self.depth -= 1
		
		elif current.type == self.token.EXPRESSION: # expressions
			if len(current.tokens) >= 2 and current.tokens[0].type == self.token.TYPE: # declarations
				variable = self.variable(self.depth, current.tokens[1].content, self.stack_pointer, self.get_type_from_token(current.tokens[0].content))
				self.stack_pointer += 8
				
				if len(current.tokens) >= 3 and current.tokens[2].type == self.token.OPERATOR and current.tokens[2].content == "=":
					extension_token = self.token(self.token.EXPRESSION)
					extension_token.tokens = current.tokens[3: len(current.tokens)]
					
					write_code = self.compile_token(extension_token, write_code)
					write_code = write_code + extension_token.reference("mov g2 ") + "\ncad bp sub %d\tmov ?ad g2\n" % variable.stack_pointer
				
				else:
					write_code = write_code + "cad bp sub %d\nmov ?ad 0\n" % (variable.stack_pointer)
				
				self.variables.append(variable)
			
			elif len(current.tokens) >= 1 and current.tokens[0].type == self.token.STATEMENT: # statments
				if current.tokens[0].content == "return": # return statement
					if len(current.tokens) >= 2:
						extension_token = self.token(self.token.EXPRESSION)
						extension_token.tokens = current.tokens[1: len(current.tokens)]
						write_code = self.compile_token(extension_token, write_code) + extension_token.reference("mov g0 ") + "\t" + self.FUNCTION_LEAVE_FORMAT
					
					else:
						write_code = write_code + "mov g0 0\n" + self.FUNCTION_LEAVE_FORMAT
			
			elif len(current.tokens) >= 3 and not len(current.tokens) % 1: # operations and chains of arithmetic with expressions
				first_operator = current.tokens[1]
				
				if first_operator.type == self.token.OPERATOR:
					for i in range(len(current.tokens) / 2 + 1):
						write_code = self.compile_branch(current.tokens[i * 2], write_code)
					
					current.stack_pointer = self.stack_pointer
					
					def get_operator_token_precedence(operator):
						if   operator.content in ["*", "/", "%"]: return 3  # mul/div/mod
						elif operator.content in ["+", "-"]:      return 4  # add/sub
						elif operator.content in ["%%", "++"]:    return 5  # string operations
						elif operator.content[-1] == "=":         return 14 # assignment
						
						elif operator.content == "":              return 17
						else:                                     return 16
					
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
						
						instruction = "mov"
						
						if   operator.content[0] == "+": instruction = "add"
						elif operator.content[0] == "*": instruction = "mul"
						elif operator.content[0] == "/": instruction = "div"
						elif operator.content[0] == "%": instruction = "mod"
						elif operator.content[0] == "-": instruction = "sub"
						
						if operator.content[-1] == "=":
							write_code = write_code + "mov g0 %s\n%s %s g0\n" % (current.tokens[min_index + 1].reference(), instruction, current.tokens[min_index - 1].reference())
						
						else:
							if not i:
								write_code = write_code + current.tokens[min_index - 1].reference("mov g0 ") + "\n"
							
							if went_left: write_code = write_code + "mov g1 g0\nmov g0 %s\n%s g0 g1\n" % (current.tokens[min_index - 1].reference(), instruction)
							else:         write_code = write_code + current.tokens[min_index + 1].reference(instruction + " g0 ") + "\n"
						
						operator.content = "\0"
					
					write_code = write_code + current.reference("mov ") + " g0\n"
					self.stack_pointer += 8
			
			elif len(current.tokens) == 1: # nested expressions
				write_code = self.compile_token(current.tokens[0], write_code)
				current.copy_reference(current.tokens[0])
		
		elif current.type == self.token.NUMBER and current.stack_pointer < 0: # number literals
			current.stack_pointer = self.stack_pointer
			write_code = write_code + current.reference("mov ") + " " + current.content + "\n"
			self.stack_pointer += 8
		
		return write_code
	
	def compile_branch(self, tree, write_code = text_section): # loop through all tokens in a branch and compile them
		if not tree:
			return
		
		write_code = self.compile_token(tree, write_code)
		
		for i in range(len(tree.tokens)):
			current = tree.tokens[i]
			write_code = self.compile_token(current, write_code)
		
		return write_code
	
	def compile_data(self, data): # check if data is already recorded and create a new entry if not
		for i in range(len(self.data)):
			self.data_section = self.data_section + "%%!amber_data_%d " % i
			
			for byte in self.data[i]:
				self.data_section = self.data_section + " x%x" % ord(byte)
			
			self.data_section = self.data_section + "%\n"
	
	def print_tokens(self, tokens, indent = 0):
		for token in tokens:
			print "%s%d\t%s" % (" " * indent, token.type, token.content)
	
	def print_tree(self, tree, indent = 0):
		for token in tree.tokens:
			print "%s%s\t%s" % (" " * indent, "->" if token.type == self.token.EXPRESSION else "-(" if token.type == self.token.FUNCTION_CALL else "{" if token.type == self.token.BLOCK else str(token.type), token.content)
			self.print_tree(token, indent + 1)
	
	def compile(self): # compile everything automatically
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
		
		return self.text_section + self.data_section

f = open("src/main.a")
code = f.read()
f.close()

compiler = amber_compiler(code)
result = compiler.compile()

f = open("main.asm", "wb")
f.write(result)
f.close()
