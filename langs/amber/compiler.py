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
		unary = None
		
		def __init__(self, type = UNKNOWN, base = None):
			if base:
				self.type = base.type
				self.tokens = base.tokens
				self.content = base.content
				self.call_token = base.call_token
				self.unary = base.unary
				self.stack_pointer = base.stack_pointer
				self.label_pointer = base.label_pointer
			
			else:
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
	function_count = 0
	inline_count = 0
	loop_count = 0
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
			
			if parsing_operator and not current in "+-*/%=<>!.^&~|?":
				self.tokens.append(self.token())
			
			if parsing_string:
				if current == '"':
					parsing_string = False
					white = True
				else: add_current = True
			elif current in " \n\t;,{}()":
				white = True
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
			
			parsing_operator = was_operator
			
			if add_current:
				self.tokens[-1].content = self.tokens[-1].content + current
			
			if white: # end of current token
				if self.tokens[-1].type != self.token.UNKNOWN or self.tokens[-1].content:
					if self.tokens[-1].type == self.token.UNKNOWN:
						if   self.tokens[-1].content in ["if", "func", "class", "return", "while", "break", "continue", "lab", "goto"]: self.tokens[-1].type = self.token.STATEMENT
						elif self.tokens[-1].content in ["int", "uint", "ftype"]: self.tokens[-1].type = self.token.TYPE
					
					self.tokens.append(self.token())
				
				white_token = True
				if current == '(':
					if len(self.tokens) >= 2 and not self.tokens[-2].type in [self.token.OPEN_PARENTHESES, self.token.OPERATOR, self.token.STATEMENT]: self.tokens[-1].type = self.token.FUNCTION_CALL # function call
					else: self.tokens[-1].type = self.token.OPEN_PARENTHESES
				
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
		unary = None
	
		previous = None
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
			elif previous and previous.type in [self.token.OPEN_PARENTHESES, self.token.OPEN_BRACES, self.token.FUNCTION_CALL, self.token.OPERATOR, self.token.STATEMENT] and current.type == self.token.OPERATOR:
				unary = current.content
				enter = 2
			else:
				branch_stack[-1].tokens.append(current)
			
			if current.type != self.token.OPERATOR and unary:
				unary = None
				exit = 2
			
			for i in range(exit):
				branch_stack.pop(-1)
			
 			for i in range(enter):
				branch_stack[-1].tokens.append(self.token(self.token.FUNCTION_CALL if current.type == self.token.FUNCTION_CALL and not i else self.token.BLOCK if block and not i else self.token.EXPRESSION))
				branch_stack[-1].unary = unary if i else None
				
				if current.type == self.token.FUNCTION_CALL and not i:
					branch_stack[-1].tokens[-1].call_token = branch_stack[-1].tokens[-2]
					branch_stack[-1].tokens.pop(-2)
				
				branch_stack.append(branch_stack[-1].tokens[-1])
			
			previous = current
	
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
					write_code = write_code + extension_token.reference("mov g2 ") + "\tcad bp sub %d\tmov ?ad g2\n" % variable.stack_pointer
				
				else:
					write_code = write_code + "cad bp sub %d\tmov ?ad 0\n" % (variable.stack_pointer)
				
				self.variables.append(variable)
			
			elif len(current.tokens) >= 1 and current.tokens[0].type == self.token.STATEMENT: # statments
				if current.tokens[0].content == "if": # if statement
					current_inline_count = self.inline_count
					self.inline_count += 1
					write_code = self.compile_token(current.tokens[1], write_code) + current.tokens[1].reference("cmp ") + " 0\tcnd zf\tjmp !amber_inline_%d_end\n" % (current_inline_count)
					write_code = self.compile_token(current.tokens[2], write_code)
					write_code = write_code + ":!amber_inline_%d_end:\n" % current_inline_count
				
				elif current.tokens[0].content == "break": # break statement
					write_code = write_code + "jmp !amber_loop_%d_end\n" % self.loop_count
				
				elif current.tokens[0].content == "continue": # continue statement
					write_code = write_code + "jmp !amber_loop_%d_condition\n" % self.loop_count
				
				elif current.tokens[0].content == "while": # while statement
					current_loop_count = self.loop_count
					self.loop_count += 1
					write_code = write_code + "jmp !amber_loop_%d_condition\t:!amber_loop_%d:\n" % (current_loop_count, current_loop_count)
					write_code = self.compile_token(current.tokens[2], write_code)
					write_code = write_code + ":!amber_loop_%d_condition:\n" % current_loop_count
					write_code = self.compile_token(current.tokens[1], write_code) + current.tokens[1].reference("cnd ") + "\tjmp !amber_loop_%d\t:!amber_loop_%d_end:\n" % (current_loop_count, current_loop_count)
				
				elif current.tokens[0].content == "func": # function statement
					variable = self.variable(self.depth, current.tokens[1].call_token.content, self.stack_pointer, self.variable.FUNCTION)
					self.variables.append(variable)
					self.stack_pointer += 8
					
					label = "!amber_function_%d" % self.function_count
					variable.label_pointer = label
					self.function_count += 1
					write_code = write_code + "cad bp sub %d\tmov ?ad %s\n" % (variable.stack_pointer, label)
					
					code = self.FUNCTION_ENTER_FORMAT % label
					for i in range(len(current.tokens[1].tokens)):
						if len(current.tokens[1].tokens[i].tokens):
							argument_variable = self.variable(self.depth, current.tokens[1].tokens[i].tokens[1].content, self.stack_pointer, self.get_type_from_token(current.tokens[1].tokens[i].tokens[0].content))
							self.variables.append(argument_variable)
							self.stack_pointer += 8
							code = code + "cad bp sub %d\tmov ?ad a%d\n" % (argument_variable.stack_pointer, i)
					
					extension_token = self.token(self.token.EXPRESSION)
					extension_token.tokens = current.tokens[2: len(current.tokens)]
					
					code = self.compile_token(extension_token, code)
					write_code = code + "mov g0 0\t" + self.FUNCTION_LEAVE_FORMAT + write_code
				
				elif current.tokens[0].content == "return": # return statement
					if len(current.tokens) >= 2:
						extension_token = self.token(self.token.EXPRESSION)
						extension_token.tokens = current.tokens[1: len(current.tokens)]
						write_code = self.compile_token(extension_token, write_code) + extension_token.reference("mov g0 ") + "\t" + self.FUNCTION_LEAVE_FORMAT
					
					else:
						write_code = write_code + "mov g0 0\n" + self.FUNCTION_LEAVE_FORMAT
				
				elif current.tokens[0].content == "lab":  write_code = write_code +    ":!amber_label_%s:\n" % current.tokens[1].content # lab statement
				elif current.tokens[0].content == "goto": write_code = write_code + "jmp !amber_label_%s\n"  % current.tokens[1].content # goto statement
			
			elif len(current.tokens) >= 3 and not len(current.tokens) % 1: # operations and chains of arithmetic with expressions
				first_operator = current.tokens[1]
				
				if first_operator.type == self.token.OPERATOR:
					for i in range(len(current.tokens) / 2 + 1):
						write_code = self.compile_branch(current.tokens[i * 2], write_code)
					
					current.stack_pointer = self.stack_pointer
					
					def get_operator_token_precedence(operator):
						if   operator.content in ["*", "/", "%"]:        return 3  # mul/div/mod
						elif operator.content in ["+", "-"]:             return 4  # add/sub
						elif operator.content in ["<<", ">>"]:           return 5  # shifts
						elif operator.content in ["<", ">", "<=", ">="]: return 6  # relational operators
						elif operator.content in ["==", "!="]:           return 7  # equivalency
						elif operator.content == "&&":                   return 8  # logical and
						elif operator.content == "||":                   return 9  # logical or
						elif operator.content in ["%%", "++"]:           return 12 # string operations
						elif operator.content == "===":                  return 13 # string equivalency
						elif operator.content[-1] == "=":                return 14 # assignment
						
						elif operator.content == "":                     return 17
						else:                                            return 16
					
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
						do_normalize = False
						
						if operator.content[0: 2] == "&&":
							do_normalize = True
							instruction = "and"
						
						elif operator.content[0: 2] == "||":
							do_normalize = True
							instruction = "or"
						
						elif operator.content[0: 2] == ">>": instruction = "ror"
						elif operator.content[0: 2] == "<<": instruction = "shl"
						
						elif operator.content[0] == "+": instruction = "add"
						elif operator.content[0] == "*": instruction = "mul"
						elif operator.content[0] == "/": instruction = "div"
						elif operator.content[0] == "%": instruction = "mod"
						elif operator.content[0] == "-": instruction = "sub"
						elif operator.content[0] == "|": instruction = "or "
						elif operator.content[0] == "&": instruction = "and"
						elif operator.content[0] == "^": instruction = "xor"
						
						def normalize(string):
							if do_normalize: return "\tmov g2 0\tcnd %s\tmov g2 1\tmov %s g2" % (string, string)
							else: return ""
						
						if operator.content[-1] == "=":
							write_code = write_code + current.tokens[min_index + 1].reference("mov g0 ") + "\t" + current.tokens[min_index - 1].reference(instruction + " ") + " g0\t" + current.tokens[min_index - 1].reference("mov g0 ") + "\n"
						
						else:
							if not i:
								write_code = write_code + current.tokens[min_index - 1].reference("mov g0 ") + normalize("g0") + "\n"
							
							if went_left: write_code = write_code + "mov g1 g0\t" + current.tokens[min_index - 1].reference("mov g0 ") + "\t%s g0 g1\n" % instruction + "\n"
							else:         write_code = write_code + current.tokens[min_index + 1].reference("mov g1 ") + normalize("g1") + "\n%s g0 g1" % instruction + "\n"
						
						operator.content = "\0"
					
					write_code = write_code + current.reference("mov ") + " g0\n"
					self.stack_pointer += 8
			
			elif len(current.tokens) == 1: # nested expressions
				write_code = self.compile_token(current.tokens[0], write_code)
				current.stack_pointer = self.stack_pointer
				write_code = write_code + current.tokens[0].reference("mov g2 ") + "\t" + current.reference("mov ") + " g2\n"
				self.stack_pointer += 8
				
				if current.unary:
					unary_operation_code = "\t"
					
					if   current.unary == "~": unary_operation_code = "\tnot g2\t"
					elif current.unary == "-": unary_operation_code = "\txor g2 x8000000000000000\tadd g2 1\t"
					elif current.unary == "?": unary_operation_code = "\tmov g2 8?g2\t"
					elif current.unary == "*": unary_operation_code = "\tmov g2 1?g2\t"
					else: print current.unary
					
					write_code = write_code + current.reference("mov g2 ") + unary_operation_code + current.reference("mov ") + " g2\n"
		
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
			
			self.data_section = self.data_section + " xa 0%\n"
	
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
