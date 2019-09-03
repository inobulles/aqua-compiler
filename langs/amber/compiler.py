
f = open("src/main.a")
code = f.read()
f.close()

# lexer

assembler = ":main:\tmov bp sp\nsub bp 1024\n"
token = ""
tokens = []
parsing_string = False

for i in range(len(code)):
	if code[i] == '"':
		parsing_string = not parsing_string
	
	elif not parsing_string and code[i] in "\t\n ":
		if token:
			tokens.append(token)
			token = ""
	
	else:
		token += code[i]

print tokens
# abstract syntax tree

tree = [[]]
branch_stack = [tree, tree[-1]]

for token in tokens:
	exit = 0
	enter = 0
	block = token in "{"
	
	if   token in ",;": exit, enter = (1, 1)
	elif token in "(":  enter = 2
	elif token in ")":  exit = 2
	elif token in "{":  enter = 2
	elif token in "}":  exit, enter = (3, 1)
	else: branch_stack[-1].append(token)
	
	for i in range(exit):
		branch_stack.pop(-1)
	
	for i in range(enter):
		branch_stack[-1].append([])
		branch_stack.append(branch_stack[-1][-1])

def print_branch(branch, indent = 0):
	for token in branch:
		if type(token) == list: print_branch(token, indent + 1)
		else: print " " * indent + token

print_branch(tree)
# compiler

variables = {}
stack_pointer = 0

def declare_variable(name):
	global variables, stack_pointer
	variables[name] = stack_pointer
	stack_pointer += 8
	return "cad bp sub %d\tmov ?ad g0\n" % variables[name]

def compile_tokens(tokens):
	assembler = ""
	
	i = 0
	while i < len(tokens):
		print tokens[i]
		
		if tokens[i] == "int": # variable declaration statement
			assembler += compile_tokens(tokens[i + 3])
			assembler += declare_variable(tokens[i + 1])
			i += 4
		
		# ~ elif tokens[i] == "return": # return statement
			# ~ assembler += return_value(tokens[i += 1]
		
		i += 1
	
	return assembler + "mov g0 ?ad\n"

assembler += compile_tokens(tree)
assembler += "mov g0 0\tret\n"
print assembler

# output

f = open("main.asm", "wb")
f.write(assembler)
f.close()
