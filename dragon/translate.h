#ifndef _TRANSLATE_H
#define _TRANSLATE_H

#include "ast.h"
#include "symbol.h"

// XHAZARD
#define ABI_POINTER_SIZE 8
#define ABI_POINTER_ALIGN 8

#define ABI_CLOSURE_SIZE (ABI_POINTER_SIZE * 2)
#define ABI_CLOSURE_ALIGN (ABI_POINTER_ALIGN * 2)

struct stab;

struct sizing {
    int32_t size, align;
};

struct rec_layout {
    struct sizing overall;
    struct ptrvec *fields;
};

struct cir_prog *translate(struct ast_program *, struct stab *);
void free_ir(struct cir_prog *);

void free_rec_layout(struct rec_layout *);
struct rec_layout *compute_rec_layout(struct stab *, struct list *);


struct tcx {

};

/*
 * Overall Design
 * ==============
 *
 * The program is represented as a digraph of functions, which is the
 * callgraph of the program. Each function is a control flow graph (CFG) of
 * basic blocks (BBS). The program after translation is in SSA form, and each
 * pass until codegen must preserve the SSA invariants. To get there, the
 * program is translated into a straightforward CFG not in SSA form. Then, the
 * algorithm of Cytron et. al is used to compute a minimal SSA and dependence
 * graph.
 *
 * Function Environments and Capture Groups
 * ========================================
 *
 * When considering the (static) call graph of the program, all functions may
 * access the local variables of its predecessors. This is somewhat
 * challenging to represent in a reusable IR such as this. The approach taken
 * is to add an argument to every function which either is or dominates a
 * function which captures nonlocals. The translator creates an aggregate type
 * for all captured locals of a function, and stores these in memory. These
 * aggregates, known as capture groups (CGs), also have a field for a pointer
 * to the parent group, as well as a unique ID identifying the function the CG
 * belongs to. Then, when a subfunction needs to access a nonlocal, it can
 * walk up the linked list of CGs until it finds the ID corresponding to the
 * function it needs, and read/write the proper field.
 *
 * CIR - Common IR
 * ===============
 *
 * CIR is the linearization of the program's CFG. Used for transformation
 * passes and codegen.
 */

enum cir_op {
    // return A from the function.
    IRET,
    // a conditional branch, "br A B C" reads "if A jump to BB B, else jump to
    // BB C"
    IBR,

    // A + B
    IADD,
    // A - B
    ISUB,
    // A * B
    IMUL,
    // A / B
    IDIV,
    // A % B
    IMOD,

    // B = ~A
    INOT,
    // A & B
    IAND,
    // A | B
    IOR,

    // load B bytes from A
    ILD,
    // A <- B
    IST,
    // space on stack for A bytes
    IALLOC,
    // A < B
    ILT,
    // A < B
    ILE,
    // A < B
    IGT,
    // A < B
    IGE,
    // A < B
    IEQ,
    // A < B
    INE,

    // PHI(a, b). Created by the SSAifier.
    IPHI,
};

enum operand_ty {
    OPER_ILIT,
    OPER_BLIT,
    OPER_FLIT,
    OPER_REG,
    OPER_LABEL
};

struct operand {
    union {
        struct insn *reg;
        struct cir_bb *label;
        bool blit;
        uint64_t ilit;
        double flit;
    };
    enum operand_ty tag;
};

enum cir_ty {
    TY_INT,
    TY_REAL,
    TY_ADDR,
};

// a simple three-address instruction
struct insn {
    enum cir_op op;
    enum cir_ty ty;
    struct operand a, b, c;
};

struct cir_bb {
    struct ptrvec *insns;
};

struct cir_func {
    struct ptrvec *args;
    struct cir_bb *entry;
};

struct cir_prog {
    struct ptrvec *funcs;
    struct cir_func *main;
};

struct cir_prog *cir_prog();
struct cir_func *cfunc_new(struct list *);
struct cir_bb *cir_bb();
struct insn *insn_new(enum cir_op, ...);
struct operand oper_new(enum operand_ty, ...);

#endif
