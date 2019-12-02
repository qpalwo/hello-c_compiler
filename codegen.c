#include <stdarg.h>
#include <stdio.h>
#include <llvm-c/Core.h>
#include <llvm-c/Analysis.h>
#include "util.h"
#include "label.h"
#include "symbol.h"
#include "absyn.h"
#include "codegen.h"

extern int CODEGEN_DEBUG;

static void CG_dumpModule(LLVMModuleRef module) {
    if (CODEGEN_DEBUG) {
        LLVMDumpModule(module);
    }
}

static void InternalError(int pos, char *message, ...) {
    va_list ap;
    fprintf(stderr, "[Code Generation Error] line %d: ", pos);
    va_start(ap, message);
    vfprintf(stderr, message, ap);
    va_end(ap);
    fprintf(stderr, "\n");
}

static CG_table TYPE_TABLE;

void CG_typeInit() {
    TYPE_TABLE = S_NewTable();
    S_Enter(TYPE_TABLE, S_Symbol("void"), LLVMVoidType());
    S_Enter(TYPE_TABLE, S_Symbol("int"), LLVMInt32Type());
    S_Enter(TYPE_TABLE, S_Symbol("float"), LLVMFloatType());
    S_Enter(TYPE_TABLE, S_Symbol("char"), LLVMInt8Type());
}

LLVMTypeRef CG_dec2TypeRef(A_tyDec dec) {
    switch (dec->kind) {
        case A_VAR_DEC: {
            LLVMTypeRef dectype = S_Find(TYPE_TABLE, dec->u.var.type);
            if (!dectype) {
                InternalError(dec->linno, "no such type defined");
            }
            return dectype;
        }
        case A_ARRAY_DEC: {
            return NULL;
            // LLVMArrayType()
        }
        case A_NULL_DEC:
            return NULL;
    }
}

LLVMTypeRef *CG_decList2TypeRef(A_tyDecList declist, int *count) {
    A_tyDecList bodydec = declist;
    int bodyCount = 0;
    while (bodydec) {
        bodyCount++;
        bodydec = bodydec->tail;
    }
    *count = bodyCount;
    LLVMTypeRef *body = checked_malloc(sizeof(*body) * bodyCount);
    bodydec = declist;
    for (int i = 0; i < bodyCount; i++, bodydec = bodydec->tail) {
        body[i] = CG_dec2TypeRef(bodydec->head);
    }
    return body;
}

S_symbol TyDecName(A_tyDec dec) {
    switch (dec->kind) {
        case A_VAR_DEC: {
            return dec->u.var.name;
        }
        case A_ARRAY_DEC: {
            return dec->u.array.name;
        }
        case A_NULL_DEC:
            return NULL;
    }
}

LLVMValueRef CG_stm(A_stm stm, LLVMModuleRef module, LLVMBuilderRef builder) {
    switch(stm->kind) {
        case A_ASSIGN_STM: {

        }
        case A_DEC_STM: {
            LLVMTypeRef dectype = CG_dec2TypeRef(stm->u.dec);
            if (!dectype) {
                InternalError(stm->linno, "create dec error");
            }
            S_symbol name = TyDecName(stm->u.dec);
            LLVMValueRef dec = LLVMBuildAlloca(builder, dectype, S_Name(name));
            Label_NewDec(name, dec);
        }
        case A_IF_STM: {

        }
        case A_WHILE_STM: {

        }
        case A_BREAK_STM: {

        }
        case A_CONTINUE_STM: {
            
        }
        case A_RETURN_STM: {

        }
    }
}

LLVMValueRef CG_stmList(A_stmList stmList, LLVMModuleRef module, LLVMBuilderRef builder) {
    LLVMValueRef fun = LLVMGetBasicBlockParent(LLVMGetInsertBlock(builder));
    LLVMBasicBlockRef block = LLVMAppendBasicBlock(fun, Label_NewFun());
    LLVMPositionBuilderAtEnd(builder, block);
    while (stmList && stmList->head) {
        CG_stm(stmList->head, module, builder);
        stmList = stmList->tail;
    }
    return block;
}

void CG_globalDec(A_globalDec globaldec, LLVMModuleRef module, LLVMBuilderRef builder) {
    switch (globaldec->kind) {
        case A_FUN: {
            LLVMTypeRef rettype = S_Find(TYPE_TABLE, globaldec->u.fun.ret);
            if (!rettype) {
                InternalError(globaldec->linno, "create fun error");
                return;
            }
            int paraCount = 0;
            LLVMTypeRef *para = CG_decList2TypeRef(globaldec->u.fun.para, &paraCount);
            LLVMTypeRef funty = LLVMFunctionType(rettype, para, paraCount, FALSE);
            if (!funty) {
                InternalError(globaldec->linno, "create fun error");
                return;
            }
            LLVMValueRef fun = LLVMAddFunction(module, S_Name(globaldec->u.fun.name), funty);
            if (!fun) {
                InternalError(globaldec->linno, "create fun error");
                return;
            }
            LLVMValueRef body = CG_stmList(globaldec->u.fun.body, module, builder);
            if (!body) {
                InternalError(globaldec->linno, "error function");
                LLVMDeleteFunction(fun);
            }
            LLVMBuildRet(builder, fun);
            if (LLVMVerifyFunction(fun, LLVMPrintMessageAction) == 1) {
                InternalError(globaldec->linno, "error function");
                LLVMDeleteFunction(fun);
            }
            break;
        }
        case A_STRUCT: {
            LLVMTypeRef struc = LLVMStructCreateNamed(LLVMGetModuleContext(module), S_Name(globaldec->u.struc.name));
            if (!struc) {
                InternalError(globaldec->linno, "create type error");
            }
            A_tyDecList bodydec = globaldec->u.struc.declist;
            int bodyCount = 0;
            LLVMTypeRef *body = CG_decList2TypeRef(globaldec->u.struc.declist, &bodyCount);
            LLVMStructSetBody(struc, body, bodyCount, FALSE);
            S_Enter(TYPE_TABLE, globaldec->u.struc.name, struc);
            break;
        }
        case A_GLOBAL_VAR: {
            LLVMTypeRef dectype = S_Find(TYPE_TABLE, globaldec->u.var.type);
            if (!dectype) {
                InternalError(globaldec->linno, "no such type defined");
            }
            LLVMValueRef globalvar = LLVMAddGlobal(module, dectype, S_Name(globaldec->u.var.name));
            if (!globalvar) {
                InternalError(globaldec->linno, "var define error");
            }
            break;
        }
    }
}

void CG_codeGen(A_globalDecList ast) {
    LLVMModuleRef module = LLVMModuleCreateWithName("main-module");
    LLVMBuilderRef builder = LLVMCreateBuilder();

    CG_typeInit();

    while (ast && ast->head) {
        CG_globalDec(ast->head, module, builder);
        ast = ast->tail;
    }
    CG_dumpModule(module);

    LLVMDisposeBuilder(builder);
    LLVMDisposeModule(module);
}
