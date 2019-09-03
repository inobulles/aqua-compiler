#ifdef ZED
#include "9cc.h"

// This pass generates zed assembly from IR.

char *regs[] = {"g1", "g2", "g3"};
char *regs8[] = {"zf", "of", "sf", "cf"};

int num_regs = sizeof(regs) / sizeof(*regs);

static char *argregs[] = {"a0", "a1", "a2", "a3"};

__attribute__((format(printf, 1, 2))) static void p(char *fmt, ...);
__attribute__((format(printf, 1, 2))) static void emit(char *fmt, ...);

#define p(...) {pn(__VA_ARGS__);printf("\n");}
static void pn(char *fmt, ...) {
	va_list ap;
  va_start(ap, fmt);
  vprintf(fmt, ap);
}

static void emit(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  printf("\t");
  vprintf(fmt, ap);
  printf("\n");
}

static char *reg(int r, int size) {
  return regs[r];
}

static char *argreg(int r, int size) {
  return argregs[r];
}

static void emit_ir(IR *ir, char *ret) {
  int r0 = ir->r0 ? ir->r0->rn : 0;
  int r1 = ir->r1 ? ir->r1->rn : 0;
  int r2 = ir->r2 ? ir->r2->rn : 0;

  switch (ir->op) {
  case IR_IMM: emit("mov %s %d", regs[r0], ir->imm); break;
  case IR_BPREL: emit("cad bp add %d\tmov %s ?ad", ir->var->offset, regs[r0]); break;
  case IR_MOV: emit("mov %s %s", regs[r0], regs[r2]); break;
  case IR_RETURN: emit("mov g0 %s", regs[r2]); emit("jmp %s", ret); break;
  case IR_CALL: for (int i = 0; i < ir->nargs; i++) pn("\tmov %s %s\t", argregs[i], regs[ir->args[i]->rn]); p("mov g0 0\tcal %s\tmov %s g0", ir->name, regs[r0]); break;
  case IR_LABEL_ADDR: emit("mov %s %s", regs[r0], ir->name); break;
  case IR_EQ: break;
  case IR_NE: break;
  case IR_LT: emit("cmp %s %s\tmov %s 0\tcnd cf\tmov %s 1", regs[ir->r1->rn], regs[ir->r2->rn], regs[ir->r0->rn], regs[ir->r0->rn]); break;
  case IR_LE: break;
  case IR_AND: emit("and %s %s", regs[r0], regs[r2]); break;
  case IR_OR: emit("or %s %s", regs[r0], regs[r2]); break;
  case IR_XOR: emit("xor %s %s", regs[r0], regs[r2]); break;
  case IR_SHL: emit("shl %s %s", regs[r0], regs[r2]); break;
  case IR_SHR: emit("shr %s %s", regs[r0], regs[r2]); break;
  case IR_JMP:
    if (ir->bbarg)
      emit("mov %s, %s", regs[ir->bb1->param->rn], regs[ir->bbarg->rn]);
    emit("jmp .L%d", ir->bb1->label);
    break;
  case IR_BR: emit("cnd %s\tjmp .L%d\tjmp .L%d\n", regs[r2], ir->bb1->label, ir->bb2->label); break;
  case IR_LOAD: emit("mov %s %c?%s", reg(r0, ir->size), ir->size == 1 ? '1' : '8', regs[r2]); break;
  case IR_LOAD_SPILL: emit("cad bp add %d\tmov %s ?ad", ir->var->offset, regs[r0]); break;
  case IR_STORE: emit("mov ?%s %s", regs[r1], reg(r2, ir->size)); break;
  case IR_STORE_ARG:
    emit("mov [bp%d], %s", ir->var->offset, argreg(ir->imm, ir->size));
    break;
  case IR_STORE_SPILL: emit("cad bp add %d\tmov ?ad %s", ir->var->offset, regs[r1]); break;
  case IR_ADD: emit("add %s, %s", regs[r0], regs[r2]); break;
  case IR_SUB: emit("sub %s, %s", regs[r0], regs[r2]); break;
  case IR_MUL:
    emit("mov rax, %s", regs[r2]);
    emit("imul %s", regs[r0]);
    emit("mov %s, rax", regs[r0]);
    break;
  case IR_DIV:
    emit("mov rax, %s", regs[r0]);
    emit("cqo");
    emit("idiv %s", regs[r2]);
    emit("mov %s, rax", regs[r0]);
    break;
  case IR_MOD:
    emit("mov rax, %s", regs[r0]);
    emit("cqo");
    emit("idiv %s", regs[r2]);
    emit("mov %s, rdx", regs[r0]);
    break;
  case IR_NOP: break;
  default: assert(0 && "unknown operator");
  }
}

void emit_code(Function *fn) {
  // Assign an offset from RBP to each local variable.
  int off = 0;
  for (int i = 0; i < fn->lvars->len; i++) {
    Var *var = fn->lvars->data[i];
    off += var->ty->size;
    off = roundup(off, var->ty->align);
    var->offset = -off;
  }

  // Emit assembly
  char *ret = format(".Lend%d", nlabel++);

  p(":%s:", fn->name);
  emit("psh bp");
  emit("mov bp, sp");
  emit("sub sp, %d", roundup(off, 16));
  emit("psh g1");
  emit("psh g2");
  emit("psh g3");

  for (int i = 0; i < fn->bbs->len; i++) {
    BB *bb = fn->bbs->data[i];
    p(":.L%d:", bb->label);
    for (int i = 0; i < bb->ir->len; i++) {
      IR *ir = bb->ir->data[i];
      emit_ir(ir, ret);
    }
  }

  p(":%s:", ret);
  emit("pop g3");
  emit("pop g2");
  emit("pop g1");
  emit("mov sp, bp");
  emit("pop bp");
  emit("ret");
}

static void emit_data(Var *var) {
  if (var->data) {
    pn("%%%s ", var->name);
    for (int i = 0; i < var->ty->size; i++)
		pn("x%x ", var->data[i]);
	p("%%");
    return;
  }
}

void gen_zed(Program *prog) {
  for (int i = 0; i < prog->gvars->len; i++)
    emit_data(prog->gvars->data[i]);

  for (int i = 0; i < prog->funcs->len; i++)
    emit_code(prog->funcs->data[i]);
}
#endif
