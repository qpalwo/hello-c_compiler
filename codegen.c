#include <assert.h>
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
typedef struct CG_baseBlockList_ * CG_baseBlockList;

LLVMBasicBlockRef CG_stmList(A_stmList stmList, LLVMModuleRef module, LLVMBuilderRef builder);
LLVMValueRef CG_exp(A_exp exp, LLVMModuleRef module, LLVMBuilderRef builder);

struct CG_baseBlockList_ {
    LLVMBasicBlockRef head;
    CG_baseBlockList tail;
};

CG_baseBlockList CG_BaseBlockList(LLVMBasicBlockRef head, CG_baseBlockList tail) {
    CG_baseBlockList list = checked_malloc(sizeof(*list));
    list->head = head;
    list->tail = tail;
    return list;
}

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

LLVMValueRef CG_ZeroValueOfType(LLVMTypeRef type) {
    switch (LLVMGetTypeKind(type)) {
        case LLVMIntegerTypeKind:
            return LLVMConstInt(LLVMInt32Type(), 0, FALSE);
        case LLVMFloatTypeKind:
            return LLVMConstReal(LLVMFloatType(), 0);
        case LLVMStructTypeKind:
        case LLVMArrayTypeKind:
        default:
            return NULL;
    }
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

LLVMValueRef *CG_callExpList2Ref(A_expList expList, LLVMModuleRef module, LLVMBuilderRef builder, int *count) {
    int num = 0;
    A_expList list = expList;
    while (expList && expList->head) {
        num++;
        expList = expList->tail;
    }
    *count = num;
    LLVMValueRef * refList = checked_malloc(sizeof(*refList) * num);
    for (int i = 0; i < num && list; i++, list = list->tail) {
        refList[i] = CG_exp(list->head, module, builder);
    }
    return refList;
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

LLVMValueRef CG_singleExp(A_exp exp, sop op, LLVMModuleRef module, LLVMBuilderRef builder) {
    LLVMValueRef expvalue = CG_exp(exp, module, builder);
    LLVMTypeRef exptype = LLVMTypeOf(expvalue);
    LLVMTypeKind exptypekind = LLVMGetTypeKind(exptype);
    switch (op) {
        case A_SPLUS: {
            if (exptypekind == LLVMIntegerTypeKind) {
                LLVMValueRef ONE = LLVMConstInt(LLVMInt8Type(), 1, FALSE);
                LLVMValueRef load = LLVMBuildLoad(builder, expvalue, Label_NewLabel("load"));
                LLVMValueRef added = LLVMBuildNSWAdd(builder, load, ONE, Label_NewLabel("add"));
                return LLVMBuildStore(builder, added, expvalue);
            } else if (exptypekind == LLVMFloatTypeKind) {
                LLVMValueRef ONE = LLVMConstReal(LLVMFloatType(), 1.0);
                return LLVMBuildFAdd(builder, expvalue, ONE, "");
            } else {
                InternalError(exp->linno, "alg exp type must be float or int");
                return NULL;
            }
        }
        case A_SMINUS: {
            if (exptypekind == LLVMIntegerTypeKind) {
                LLVMValueRef MONE = LLVMConstInt(LLVMInt8Type(), -1, FALSE);
                return LLVMBuildAdd(builder, expvalue, MONE, "");
            } else if (exptypekind == LLVMFloatTypeKind) {
                LLVMValueRef MONE = LLVMConstReal(LLVMFloatType(), -1.0);
                return LLVMBuildFAdd(builder, expvalue, MONE, "");
            } else {
                InternalError(exp->linno, "alg exp type must be float or int");
                return NULL;
            }
        }
        case A_NEGATIVE: {
            return LLVMBuildNeg(builder, expvalue, "");
        }
        case A_POSITIVE: {
            return expvalue;
        }
        case A_NOT: {
            return LLVMBuildNot(builder, expvalue, "");
        }
    }

}

LLVMValueRef CG_doubleExp(A_exp le, A_exp re, dop op, LLVMModuleRef module, LLVMBuilderRef builder) {
    LLVMValueRef lexpvalue = CG_exp(le, module, builder);
    LLVMTypeRef lexptype = LLVMTypeOf(lexpvalue);
    LLVMTypeKind lexptypekind = LLVMGetTypeKind(lexptype);
    LLVMValueRef rexpvalue = CG_exp(re, module, builder);
    LLVMTypeRef rexptype = LLVMTypeOf(rexpvalue);
    LLVMTypeKind rexptypekind = LLVMGetTypeKind(rexptype);
    LLVMValueRef lfv = NULL, rfv = NULL;
    if (lexptypekind == LLVMFloatTypeKind || rexptypekind == LLVMFloatTypeKind) {
        if (lexptypekind == LLVMFloatTypeKind) {
            lfv = lexpvalue;
        } else {
            lfv = LLVMBuildIntCast2(builder, lexpvalue, LLVMFloatType(), TRUE, "");
        }
        if (rexptypekind == LLVMFloatTypeKind) {
            rfv = lexpvalue;
        } else {
            rfv = LLVMBuildIntCast2(builder, rexpvalue, LLVMFloatType(), TRUE, "");
        }
        assert(lfv && lfv);
    } 
    LLVMIntPredicate compareOP = 0;
    LLVMRealPredicate compareFOP = 0;
    switch (op) {
        case A_PLUS: {
            if (!lfv) {
                return LLVMBuildAdd(builder, lexpvalue, rexpvalue, "");
            } else {
                return LLVMBuildFAdd(builder, lfv, rfv, "");
            }
        }
        case A_MINUS: {
            if (!lfv) {
                return LLVMBuildSub(builder, lexpvalue, rexpvalue, "");
            } else {
                return LLVMBuildFSub(builder, lfv, rfv, "");
            }
        }
        case A_TIMES: {
            if (!lfv) {
                return LLVMBuildMul(builder, lexpvalue, rexpvalue, "");
            } else {
                return LLVMBuildFMul(builder, lfv, rfv, "");
            }
        }
        case A_DIVIDE: {
            if (!lfv) {
                return LLVMBuildSDiv(builder, lexpvalue, rexpvalue, "");
            } else {
                return LLVMBuildFDiv(builder, lfv, rfv, "");
            }
        }
        case A_EQ:
            compareOP = LLVMIntEQ;
            compareFOP = LLVMRealOEQ;
            break;
        case A_NEQ:
            compareOP = LLVMIntNE;
            compareFOP = LLVMRealONE;
            break;
        case A_GT:
            compareOP = LLVMIntSGT;
            compareFOP = LLVMRealOGT;
            break;
        case A_GE:
            compareOP = LLVMIntSGE;
            compareFOP = LLVMRealOGE;
            break;
        case A_LT:
            compareOP = LLVMIntSLT;
            compareFOP = LLVMRealOLT;
            break;
        case A_LE:
            compareOP = LLVMIntSLE;
            compareFOP = LLVMRealOLE;
            break;
        case A_BITAND: {
            if (!lfv) {
                InternalError(le->linno, "logic algo must be int");
                return LLVMConstInt(LLVMInt8Type(), 0, FALSE);
            } else {
                return LLVMBuildAnd(builder, lfv, rfv, "");
            }
        }
        case A_BITOR: {
            if (!lfv) {
                InternalError(le->linno, "logic algo must be int");
                return LLVMConstInt(LLVMInt8Type(), 0, FALSE);
            } else {
                return LLVMBuildOr(builder, lfv, rfv, "");
            }
        }
        case A_AND:
        case A_OR:;
    }
    if (lfv) {
        return LLVMBuildFCmp(builder, compareFOP, lfv, rfv, "");
    } else {
        return LLVMBuildICmp(builder, compareOP, lexpvalue, rexpvalue, "");
    }
}

LLVMValueRef CG_var(A_var var, LLVMModuleRef module, LLVMBuilderRef builder) {
    switch (var->kind) {
        case A_SYMBOL_VAR: {
            return Label_FindDec(var->u.symbol);
        }
        case A_ARRAY_VAR: {

        }
        case A_STRUCT_VAR: {

        }
    }

}

LLVMValueRef CG_exp(A_exp exp, LLVMModuleRef module, LLVMBuilderRef builder) {
    switch (exp->kind) {
        case A_CONST: {
            switch (exp->u.cons.kind) {
                case A_INT:
                    return LLVMConstInt(LLVMInt32Type(), exp->u.cons.u.inum, FALSE);
                case A_CHAR:
                    return LLVMConstInt(LLVMInt8Type(), exp->u.cons.u.cnum, FALSE);
                case A_FLOAT:
                    return LLVMConstReal(LLVMFloatType(), exp->u.cons.u.fnum);
                case A_VAR:
                    return CG_var(exp->u.cons.u.var, module, builder);
            }
        }
        case A_CALL: {
            LLVMValueRef fun = Label_FindDec(exp->u.call.name);
            if (!fun) {
                InternalError(exp->linno, "find call fun error");
                return NULL;
            }
            int paraCount = 0;
            LLVMValueRef * paras = CG_callExpList2Ref(exp->u.call.para, module, builder, &paraCount);
            LLVMValueRef call = LLVMBuildCall(builder, fun, paras, paraCount, "");
            if (!call) {
                InternalError(exp->linno, "create call instrction error");
                return NULL;
            }
            return call;
        }
        case A_DOUBLE_EXP: {
            return CG_doubleExp(
                exp->u.doublexp.left, exp->u.doublexp.right, exp->u.doublexp.op, module, builder);
        }
        case A_SINGLE_EXP: {
            return CG_singleExp(exp->u.singexp.exp, exp->u.singexp.op, module, builder);
        }
    }
}

LLVMBasicBlockRef CG_stm(A_stm stm, LLVMModuleRef module, LLVMBuilderRef builder) {
    switch(stm->kind) {
        case A_ASSIGN_STM: {
            LLVMValueRef exp = CG_exp(stm->u.assign.exp, module, builder);
            if (!exp) {
                InternalError(stm->linno, "create assign stm exp error");
            }
            LLVMValueRef value = CG_var(stm->u.assign.symbol, module, builder);
            if (!value) {
                InternalError(stm->linno, "get assign stm value error");
            }
            LLVMBuildStore(builder, exp, value);
            return NULL;
        }
        case A_DEC_STM: {
            LLVMTypeRef dectype = CG_dec2TypeRef(stm->u.dec);
            if (!dectype) {
                InternalError(stm->linno, "create dec error");
            }
            S_symbol name = TyDecName(stm->u.dec);
            LLVMValueRef dec = LLVMBuildAlloca(builder, dectype, S_Name(name));
            Label_NewDec(name, dec);
            return NULL;
        }
        case A_IF_STM: {
            LLVMBasicBlockRef nowBlock = LLVMGetInsertBlock(builder);
            LLVMValueRef fun = LLVMGetBasicBlockParent(nowBlock);
            LLVMValueRef cond = CG_exp(stm->u.iff.test, module, builder);
            LLVMValueRef INTZERO = LLVMConstInt(LLVMInt8Type(), 0, FALSE);
            cond = LLVMBuildICmp(builder, LLVMIntNE, cond, INTZERO, Label_NewLabel("ifcond"));
            LLVMBasicBlockRef then = LLVMAppendBasicBlock(fun, Label_NewLabel("if.then"));
            LLVMBasicBlockRef elsee = LLVMAppendBasicBlock(fun, Label_NewLabel("if.else"));
            LLVMBasicBlockRef end = LLVMAppendBasicBlock(fun, Label_NewLabel("if.end"));
            LLVMPositionBuilderAtEnd(builder, nowBlock);
            LLVMValueRef condins = LLVMBuildCondBr(builder, cond, then, elsee);
            LLVMPositionBuilderAtEnd(builder, then);
            Label_NewScope();
            LLVMBasicBlockRef thenEndBlock = CG_stmList(stm->u.iff.iff, module, builder);
            if (!thenEndBlock) {
                LLVMBuildBr(builder, end);
            }
            Label_EndScope();
            LLVMPositionBuilderAtEnd(builder, elsee);
            Label_NewScope();
            LLVMBasicBlockRef elseEndBlock = CG_stmList(stm->u.iff.elsee, module, builder);
            if (!elseEndBlock) {
                LLVMBuildBr(builder, end);
            }
            Label_EndScope();
            // handle nested end block
            if (thenEndBlock) {
                LLVMPositionBuilderAtEnd(builder, thenEndBlock);
                LLVMBuildBr(builder, end);
            }
            if (elseEndBlock) {
                LLVMPositionBuilderAtEnd(builder, elseEndBlock);
                LLVMBuildBr(builder, end);
            }
            // set builder pointer at next BaseBlock
            LLVMPositionBuilderAtEnd(builder, end);
            return end;
        }
        case A_WHILE_STM: {

        }
        case A_BREAK_STM: {

        }
        case A_CONTINUE_STM: {

        }
        case A_RETURN_STM: {

        }
        return NULL;
    }
}

LLVMBasicBlockRef CG_stmList(A_stmList stmList, LLVMModuleRef module, LLVMBuilderRef builder) {
    CG_baseBlockList endblock = NULL;
    while (stmList && stmList->head) {
        LLVMBasicBlockRef end = CG_stm(stmList->head, module, builder);
        if (end) {
            endblock = CG_BaseBlockList(end, endblock);
        }
        stmList = stmList->tail;
    }
    // merge end block
    if (endblock && endblock->head) {
        LLVMValueRef fun = LLVMGetBasicBlockParent(LLVMGetInsertBlock(builder));
        LLVMBasicBlockRef endbb = LLVMAppendBasicBlock(fun, Label_NewLabel("end"));
        while (endblock && endblock->head) {
            LLVMPositionBuilderAtEnd(builder, endblock->head);
            LLVMBuildBr(builder, endbb);
            endblock = endblock->tail;
        }
        return endbb;
    } else {
        return NULL;
    }
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

            LLVMBasicBlockRef block = LLVMAppendBasicBlock(fun, Label_NewFun(globaldec->u.fun.name, fun));
            LLVMPositionBuilderAtEnd(builder, block);
            // alloc para
            A_tyDecList paralist = globaldec->u.fun.para;
            LLVMTypeRef * typeList = checked_malloc(sizeof(*typeList) * paraCount);
            LLVMGetParamTypes(funty, typeList);
            for (int i = 0; i < paraCount && paralist; i++, paralist = paralist->tail) {
                S_symbol decName = TyDecName(paralist->head);
                LLVMValueRef paravalue = LLVMBuildAlloca(builder, typeList[i], S_Name(decName));
                LLVMBuildStore(builder, LLVMGetParam(fun, i), paravalue);
                Label_NewDec(decName, paravalue);
            }
            // build fun body
            LLVMBasicBlockRef bodyend = CG_stmList(globaldec->u.fun.body, module, builder);
            if (bodyend) {
                LLVMPositionBuilderAtEnd(builder, bodyend);
            }
            LLVMValueRef retv = CG_ZeroValueOfType(rettype);
            if (retv) {
                LLVMBuildRet(builder, retv);
            } else {
                LLVMBuildRetVoid(builder);
            }
            Label_EndFun();
            // if (LLVMVerifyFunction(fun, LLVMPrintMessageAction) == 1) {
            //     InternalError(globaldec->linno, "error function");
            //     LLVMDeleteFunction(fun);
            // }
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
    Label_InitTable();
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