#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>

int token;                    //current token
int token_val;                //value of current token
char *src, *old_src;          //pointer to source code string 
int poolsize;                 //default size of text/data/stack
int line;                     //line number
int *text,                    //text segment
    *old_text,                //for dump text segment
    *stack;                   //stack
char *data;                   //data segment
int *pc, *bp, *sp, ax, cycle; //虛擬機暫存器
int *current_id,              //current parsed ID
    *symbols;                 //symbol table
int *idmain;                  //the main function

//instructions
enum { LEA ,IMM ,JMP ,CALL,JZ  ,JNZ ,ENT ,ADJ ,LEV ,LI  ,LC  ,SI  ,SC  ,PUSH,
       OR  ,XOR ,AND ,EQ  ,NE  ,LT  ,GT  ,LE  ,GE  ,SHL ,SHR ,ADD ,SUB ,MUL ,DIV ,MOD ,
       OPEN,READ,CLOS,PRTF,MALC,MSET,MCMP,EXIT };

//tokens and classes (operators last and in precedence order)
enum {
  Num = 128, Fun, Sys, Glo, Loc, Id,
  Char, Else, Enum, If, Int, Return, Sizeof, While,
  Assign, Cond, Lor, Lan, Or, Xor, And, Eq, Ne, Lt, Gt, Le, Ge, Shl, Shr, Add, Sub, Mul, Div, Mod, Inc, Dec, Brak
};

//fields of identifier
enum {Token, Hash, Name, Type, Class, Value, BType, BClass, BValue, IdSize};

//types of variable/function
enum { CHAR, INT, PTR };

int basetype;                 //the type of a declaration, make it global for convenience
int expr_type;                //the type of an expression

int index_of_bp;              //index of bp pointer on stack

void next() {
  char *last_pos;
  int hash;

  while (token = *src) {
    ++src;
    
    //parse token here
    if (token == '\n') {
      ++line;
    }

    else if (token == '#') {
      //skip macro
      while (*src != 0 && *src != '\n') {
        src++;
      }
    }

    else if ((token >= 'a' && token <= 'z') || (token >= 'A' && token <= 'Z') || (token == '_')) {
      //parse identifier
      last_pos = src - 1;
      hash = token;

      while ((*src >= 'a' && *src <= 'z') || (*src >= 'A' && *src <= 'Z') || (*src >= '0' && *src <= '9') || (*src == '_')) {
        hash = hash * 147 + *src;
        src++;
      }
      //look for existing identifier, linear search
      current_id = symbols;
      while (current_id[Token]) {
        if (current_id[Hash] == hash && !memcmp((char *)current_id[Name], last_pos, src - last_pos)) {
          token = current_id[Token];
          return;
        }
        current_id = current_id + IdSize;
      }
      
      //store new ID
      current_id[Name] = (int)last_pos;
      current_id[Hash] = hash;
      token = current_id[Token] = Id;
      return;
    }
    else if (token >= '0' && token <= '9') {
      //parse number, three kinds: dec hex oct
      token_val = token - '0';
      if (token_val > 0) {
        //dec, starts with [1-9]
        while (*src >= '0' && *src <= '9') {
          token_val = token_val*10 + *src++ - '0';
        }
      } 
      else {
        //starts with 0
        if (*src == 'x' || *src == 'X') {
          //hex
          token = *++src;
          while ((token >= '0' && token <= '9') || (token >= 'a' && token <= 'f') || (token >= 'A' && token <= 'F')) {
            token_val = token_val * 16 + (token & 15) + (token >= 'A' ? 9 : 0);
            token = *++src;
          }
        } 
        else {
          //oct
          while (*src >= '0' && *src <= '7') {
            token_val = token_val*8 + *src++ - '0';
          }
        }
      }

      token = Num;
      return;
    }
    else if (token == '"' || token == '\'') {
      //parse string literal, currently, the only supported escape
      //character is '\n', store the string literal into data
      last_pos = data;
      while (*src != 0 && *src != token) {
        token_val = *src++;
        if (token_val == '\\') {
          //escape character
          token_val = *src++;
          if (token_val == 'n') {
            token_val = '\n';
          }
        }
        if (token == '"') {
          *data++ = token_val;
        }
      }
      src++;
      //if it is a single character, return Num token
      if (token == '"') {
        token_val = (int)last_pos;
      }
      else {
        token = Num;
      }
      return;
    }
    else if (token == '/') {
      if (*src == '/') {
        //skip comment
        while (*src != 0 && *src != '\n') {
          ++src;
        }
      }
      else {
        //divide operator
        token = Div;
        return;
      }
    }
    else if (token == '=') {
      //parse '==' and '='
      if (*src == '=') {
        src ++;
        token = Eq;
      }
      else {
        token = Assign;
      }
      return;
    }
    else if (token == '+') {
      //parse '+' and '++'
      if (*src == '+') {
        src ++;
        token = Inc;
      }
      else {
        token = Add;
      }
      return;
    }
    else if (token == '-') {
      //parse '-' and '--'
      if (*src == '-') {
        src ++;
        token = Dec;
      }
      else {
        token = Sub;
      }
      return;
    }
    else if (token == '!') {
      //parse '!='
      if (*src == '=') {
        src++;
        token = Ne;
      }
      return;
    }
    else if (token == '<') {
      //parse '<=', '<<' or '<'
      if (*src == '=') {
        src ++;
        token = Le;
      }
      else if (*src == '<') {
        src ++;
        token = Shl;
      }
      else {
        token = Lt;
      }
      return;
    }
    else if (token == '>') {
      //parse '>=', '>>' or '>'
      if (*src == '=') {
        src ++;
        token = Ge;
      }
      else if (*src == '>') {
        src ++;
        token = Shr;
      }
      else {
        token = Gt;
      }
      return;
    }
    else if (token =='|') {
      //parse '|' and '||'
      if (*src == '|') {
        src ++;
        token = Lor;
      }
      else {
        token = Or;
      }
      return;
    }
    else if (token == '&') {
      //parse '&' and '&&'
      if (*src == '&') {
        src ++;
        token = Lan;
      }
      else {
        token = And;
      }
      return;
    }
    else if (token == '^') {
      token = Xor;
      return;
    }
    else if (token == '%') {
      token = Mod;
      return;
    }
    else if (token == '*') {
      token = Mul;
      return;
    }
    else if (token == '[') {
      token = Brak;
      return;
    }
    else if (token == '?') {
      token = Cond;
      return;
    }
    else if (token == '~' || token == ';' || token == '{' || token == '}' || token == '(' || token == ')' || token == ']' || token == ',' || token == ':') {
      //directly return the character as token
      return;
    }
  }
  return;
}

void match(int tk) {
  if (token == tk) {
    next();
  }
  else {
    printf("%d: expected token: %c %c\n", line, tk, token);
    exit(-1);
  }
}

void expression(int level) {
  int *id;
  int tmp;
  int *addr;
  {
    if (!token) {
      printf("%d: unexpected token EOF of expression\n", line);
      exit(-1);
    }
    if (token == Num) {
      match(Num);

      *++text = IMM;
      *++text = token_val;
      expr_type = INT;
    }
    else if (token == '"') {
      *++text = IMM;
      *++text = token_val;

      match('"');
      while (token == '"') {
        match('"');
      }

      data = (char *)(((int)data + sizeof(int)) & (-sizeof(int)));
      expr_type = PTR;
    }
    else if (token == Sizeof) {
      match(Sizeof);
      match('(');
      expr_type = INT;

      if (token == Int) {
        match(Int);
      }
      else if (token == Char) {
        match(Char);
        expr_type = CHAR;
      }

      while (token == Mul) {
        match(Mul);
        expr_type = expr_type + PTR;
      }

      match(')');

      *++text = IMM;
      *++text = (expr_type == CHAR) ? sizeof(char) : sizeof(int);

      expr_type = INT;
    }
    else if (token == Id) {
      match(Id);

      id = current_id;

      if (token == '(') {
        match('(');

        tmp = 0;
        while (token != ')') {
          expression(Assign);
          *++text = PUSH;
          tmp ++;

          if (token == ',') {
            match(',');
          }
        }
        match(')');
        if (id[Class] == Sys) {
          *++text = id[Value];
        }
        else if (id[Class] == Fun) {
          *++text = CALL;
          *++text = id[Value];
        }
        else {
          printf("%d: bad function call\n", line);
          exit(-1);
        }
        if (tmp > 0) {
          *++text = ADJ;
          *++text = tmp;
        }
        expr_type = id[Type];
      }
      else if (id[Class] == Num) {
        *++text = IMM;
        *++text = id[Value];
        expr_type = INT;
      }
      else {
        if (id[Class] == Loc) {
          *++text = LEA;
          *++text = index_of_bp - id[Value];
        }
        else if (id[Class] == Glo) {
          *++text = IMM;
          *++text = id[Value];
        }
        else {
          printf("%d: undefined variable\n", line);
          exit(-1);
        }
        expr_type = id[Type];
        *++text = (expr_type == Char) ? LC : LI;
      }
    }
    else if (token == '(') {
      match('(');
      if (token == Int || token == Char) {
        tmp = (token == Char) ? CHAR : INT;
        match(token);
        while (token == Mul) {
          match(Mul);
          tmp = tmp + PTR;
        }
        match(')');
        expression(Inc);
        expr_type = tmp;
      }
      else {
        expression(Assign);
        match(')');
      }
    }
    else if (token == Mul) {
      match(Mul);
      expression(Inc);
      if (expr_type >= PTR) {
        expr_type = expr_type - PTR;
      }
      else {
        printf("%d: bad dereferencen\n", line);
        exit(-1);
      }
      *++text = (expr_type == CHAR) ? LC : LI;
    }
    else if (token == And) {
      match(And);
      expression(Inc);
      if (*text == LC || *text == LI) {
        text --;
      }
      else {
        printf("%d: bad address\n", line);
        exit(-1);
      }
      expr_type = expr_type + PTR;
    }
    else if (token == '!') {
      match('!');
      expression(Inc);
      *++text = PUSH;
      *++text = IMM;
      *++text = 0;
      *++text = EQ;
      expr_type = INT;
    }
    else if (token == '~') {
      match('~');
      expression(Inc);
      *++text = PUSH;
      *++text = IMM;
      *++text = -1;
      *++text = XOR;
      expr_type = INT;
    }
    else if (token == Add) {
      match(Add);
      expression(Inc);
      expr_type = INT;
    }
    else if (token == Sub) {
      match(Sub);
      if (token == Num) {
        *++text = IMM;
        *++text = -token_val;
        match(Num);
      }
      else {
        *++text = IMM;
        *++text = -1;
        *++text = PUSH;
        expression(Inc);
        *++text = MUL;
      }
      expr_type = INT;
    }
    else if (token == Inc || token == Dec) {
      tmp = token;
      match(token);
      expression(Inc);
      if (*text == LC) {
        *text = PUSH;
        *++text = LC;
      }
      else if (*text == LI) {
        *text = PUSH;
        *++text = LI;
      }
      else {
        printf("%d: bad lvalue of pre-increment\n", line);
        exit(-1);
      }
      *++text = PUSH;
      *++text = IMM;
      *++text = (expr_type > PTR) ? sizeof(int) : sizeof(char);
      *++text = (tmp == Inc) ? ADD : SUB;
      *++text = (expr_type == CHAR) ? SC : SI;
    }
    else {
      printf("%d: bad expression : %c\n", line, token);
      exit(-1);
    }
  }

  {
    while (token >= level) {
      tmp = expr_type;
      if (token == Assign) {
        match(Assign);
        if (*text == LC || *text == LI) {
          *text = PUSH;
        }
        else {
          printf("%d: bad lvalue in assignment\n", line);
          exit(-1);
        }
        expression(Assign);
        expr_type = tmp;
        *++text = (expr_type == CHAR) ? SC : SI;
      }
      else if (token == Cond) {
        match(Cond);
        *++text = JZ;
        addr = ++text;
        expression(Assign);
        if (token == ':') {
          match(':');
        }
        else {
          printf("%d: missing colon in conditional\n", line);
          exit(-1);
        }
        *addr = (int)(text + 3);
        *++text = JMP;
        addr = ++text;
        expression(Cond);
        *addr = (int)(text + 1);
      }
      else if (token == Lor) {
        match(Lor);
        *++text = JNZ;
        addr = ++text;
        expression(Lan);
        *addr = (int)(text + 1);
        expr_type = INT;
      }
      else if (token == Lan) {
        match(Lan);
        *++text = JZ;
        addr = ++text;
        expression(Or);
        *addr = (int)(text + 1);
        expr_type = INT;
      }
      else if (token == Or) {
        match(Or);
        *++text = PUSH;
        expression(Xor);
        *++text = OR;
        expr_type = INT;
      }
      else if (token == Xor) {
        match(Xor);
        *++text = PUSH;
        expression(And);
        *++text = XOR;
        expr_type = INT;
      }
      else if (token == And) {
        match(And);
        *++text = PUSH;
        expression(Eq);
        *++text = AND;
        expr_type = INT;
      }
      else if (token == Eq) {
        match(Eq);
        *++text = PUSH;
        expression(Ne);
        *++text = EQ;
        expr_type = INT;
      }
      else if (token == Ne) {
        match(Ne);
        *++text = PUSH;
        expression(Lt);
        *++text = NE;
        expr_type = INT;
      }
      else if (token == Lt) {
        match(Lt);
        *++text = PUSH;
        expression(Shl);
        *++text = LT;
        expr_type = INT;
      }
      else if (token == Gt) {
        match(Gt);
        *++text = PUSH;
        expression(Shl);
        *++text = GT;
        expr_type = INT;
      }
      else if (token == Le) {
        match(Le);
        *++text = PUSH;
        expression(Shl);
        *++text = LE;
        expr_type = INT;
      }
      else if (token == Ge) {
        match(Ge);
        *++text = PUSH;
        expression(Shl);
        *++text = GE;
        expr_type = INT;
      }
      else if (token == Shl) {
        match(Shl);
        *++text = PUSH;
        expression(Add);
        *++text = SHL;
        expr_type = INT;
      }
      else if (token == Shr) {
        match(Shr);
        *++text = PUSH;
        expression(Add);
        *++text = SHR;
        expr_type = INT;
      }
      else if (token == Add) {
        match(Add);
        *++text = PUSH;
        expression(Mul);
        expr_type = tmp;
        if (expr_type > PTR) {
          *++text = PUSH;
          *++text = IMM;
          *++text = sizeof(int);
          *++text = MUL;
        }
        *++text = ADD;
      }
      else if (token == Sub) {
        match(Sub);
        *++text = PUSH;
        expression(Mul);
        if (tmp > PTR && tmp == expr_type) {
          *++text = SUB;
          *++text = PUSH;
          *++text = IMM;
          *++text = sizeof(int);
          *++text = DIV;
          expr_type = INT;
        }
        else if (tmp > PTR) {
          *++text = PUSH;
          *++text = IMM;
          *++text = sizeof(int);
          *++text = MUL;
          *++text = SUB;
          expr_type = tmp;
        }
        else {
          *++text = SUB;
          expr_type = tmp;
        }
      }
      else if (token == Mul) {
        match(Mul);
        *++text = PUSH;
        expression(Inc);
        *++text = MUL;
        expr_type = tmp;
      }
      else if (token == Div) {
        match(Div);
        *++text = PUSH;
        expression(Inc);
        *++text = DIV;
        expr_type = tmp;
      }
      else if (token == Mod) {
        match(Mod);
        *++text = PUSH;
        expression(Inc);
        *++text = MOD;
        expr_type = tmp;
      }
      else if (token == Inc || token == Dec) {
        if (*text == LI) {
          *text = PUSH;
          *++text = LI;
        }
        else if (*text == LC) {
          *text = PUSH;
          *++text = LC;
        }
        else {
          printf("%d: bad value in increment\n", line);
          exit(-1);
        }
        *++text = PUSH;
        *++text = IMM;
        *++text = (expr_type > PTR) ? sizeof(int) : sizeof(char);
        *++text = (token == Inc) ? ADD : SUB;
        *++text = (expr_type == CHAR) ? SC : SI;
        *++text = PUSH;
        *++text = IMM;
        *++text = (expr_type > PTR) ? sizeof(int) : sizeof(char);
        *++text = (token == Inc) ? SUB : ADD;
        match(token);
      }
      else if (token == Brak) {
        match(Brak);
        *++text = PUSH;
        expression(Assign);
        match(']');
        if (tmp > PTR) {
          *++text = PUSH;
          *++text = IMM;
          *++text = sizeof(int);
          *++text = MUL;
        }
        else if (tmp < PTR) {
          printf("%d: pointer type expected\n", line);
          exit(-1);
        }
        expr_type = tmp - PTR;
        *++text = ADD;
        *++text = (expr_type == CHAR) ? LC : LT;
      }
      else {
        printf("%d: compiler error, token = %d\n", line, token);
        exit(-1);
      }
    }
  }
}

void statement() {
  int *a, *b; //branch control

  if (token == If) {
    match(If);
    match('(');
    expression(Assign);
    match(')');

    *++text = JZ;
    b = ++text;

    statement();
    if (token == Else) {
      match(Else);
      *b = (int)(text + 3);
      *++text = JMP;
      b = ++text;

      statement();
    }
    *b = (int)(text + 1);
  }
  else if (token == While) {
    match(While);

    a = text + 1;
    
    match('(');
    expression(Assign);
    match(')');

    *++text = JZ;
    b = ++text;

    statement();

    *++text = JMP;
    *++text = (int)a;
    *b = (int)(text + 1);
  }
  else if (token == '{') {
    match('{');
    while (token != '}') {
      statement();
    }
    match('}');
  }
  else if (token == Return) {
    match(Return);

    if (token != ';') {
      expression(Assign);
    }
    match(';');

    *++text = LEV;
  }
  else if (token == ';') {
    match(';');
  }
  else {
    expression(Assign);
    match(';');
  }
}

void function_parameter() {
  int type;
  int params;
  params = 0;
  while (token != ')') {
    type = INT;
    if (token == Int) {
      match(Int);
    }
    else if (token == Char) {
      type = CHAR;
      match(Char);
    }

    //pointer type
    while (token == Mul) {
      match(Mul);
      type = type + PTR;
    }

    //parameter name
    if (token != Id) {
      printf("%d\n%d\n", token, Id);
      printf("%d: bad parameter declaration\n", line);
      exit(-1);
    }
    if (current_id[Class] == Loc) {
      printf("%d: duplicate parameter declaration\n", line);
      exit(-1);
    }
    match(Id);
    //store the local variable
    current_id[BClass] = current_id[Class]; current_id[Class] = Loc;
    current_id[BType] = current_id[Type]; current_id[Type] = type;
    current_id[BValue] = current_id[Value]; current_id[Value] = params++; //index of current parameter
    if (token == ',') {
      match(',');
    }
  }
  index_of_bp = params+1;
}

void function_body() {
  int pos_local; //position of local variables on the stack
  int type;
  pos_local = index_of_bp;

  while (token == Int || token == Char) {
    basetype = (token == Int) ? INT : CHAR;
    match(token);

    while (token != ';') {
      type = basetype;
      while (token == Mul) {
        match(Mul);
        type = type + PTR;
      }
  
      if (token != Id) {
        printf("%d: bad local declaration\n", line);
        exit(-1);
      }
      if (current_id[Class] == Loc) {
        printf("%d: duplicate local declaration\n", line);
        exit(-1);
      }
      match(Id);
  
      //store the local variable
      current_id[BClass] = current_id[Class]; current_id[Class] = Loc;
      current_id[BType] = current_id[Type]; current_id[Type] = type;
      current_id[BValue] = current_id[Value]; current_id[Value] = ++pos_local; //index of current parameter
  
      if (token == ',') {
        match(',');
      }
    }
    match(';');
  }
  *++text = ENT;
  *++text = pos_local - index_of_bp;

  while (token != '}') {
    statement();
  }

  *++text = LEV;
}

void function_declaration() {
  match('(');
  function_parameter();
  match(')');
  match('{');
  function_body();
  current_id = symbols;
  while (current_id[Token]) {
    if (current_id[Class] == Loc) {
      current_id[Class] = current_id[BClass];
      current_id[Type]  = current_id[BType];
      current_id[Value] = current_id[BValue];
    }
    current_id = current_id + IdSize;
  }
}

void enum_declaration() {
  int i;
  i = 0;
  while (token != '}') {
    if (token != Id) {
      printf("%d: bad enum identifier %d\n", line, token);
      exit(-1);
    }
    next();
    if (token == Assign) {
      next();
      if (token != Num) {
        printf("%d: bad enum initializer\n", line);
        exit(-1);
      }
      i = token_val;
      next();
    }
    current_id[Class] = Num;
    current_id[Type] = INT;
    current_id[Value] = i++;

    if (token == ',') {
      next();
    }
  }
}

void global_declaration() {
  int type; //tmp, actual type for variable
  int i;    //tmp

  basetype = INT;
  
  //parse enum, this should be treated alone
  if (token == Enum) {
    //enum [id] { a = 10, b = 20, ... }
    match(Enum);
    if (token != '{') {
      match(Id);
    }
    if (token == '{') {
      match('{');
      enum_declaration();
      match('}');
    }
    match(';');
    return;
  }
  if (token == Int) {
    match(Int);
  }
  else if (token == Char) {
    match(Char);
    basetype = CHAR;
  }
  while (token != ';' && token != '}') {
    type = basetype;
    while(token == Mul) {
      match(Mul);
      type = type + PTR;
    }
    if (token != Id) {
      printf("%d: bad global declaration\n", line);
      exit(-1);
    }
    if (current_id[Class]) {
      printf("%d: duplicate global declaration\n", line);
      exit(-1);
    }
    match(Id);
    current_id[Type] = type;

    if (token == '(') {
      current_id[Class] = Fun;
      current_id[Value] = (int)(text + 1);
      function_declaration();
    }
    else {
      current_id[Class] = Glo;
      current_id[Value] = (int)data;
      data = data + sizeof(int);
    }
    if (token == ',') {
      match(',');
    }
  }
  next();
}

void program() {
  next();
  while (token > 0) {
    global_declaration();
  }
}

int eval() {
  int op, *tmp;
  while (1) {
    op = *pc++;

    if (op == IMM)     {ax = *pc++;}               //load immediate value to ax
    else if (op == LC) {ax = *(char *)ax;}         //load character to ax, address in ax
    else if (op == LI) {ax = *(int *)ax;}          //load integer to ax, address in ax
    else if (op == SC) {ax = *(char *)*sp++ = ax;} //save character to address, value in ax, address on stack
    else if (op == SI) {*(int *)*sp++ = ax;}       //save integer to address, value in ax, address on stack
    
    else if (op == PUSH) {*--sp = ax;}             //push the value of ax onto the stack
    else if (op == JMP) {pc = (int *)*pc;}         //jump to the address

    else if (op == JZ) {pc = ax ? pc +1 : (int *)*pc;} //jump if ax is zero
    else if (op == JNZ) {pc = ax ? (int *)*pc : pc +1;}//jump if ax is not zero

    else if (op == CALL) {*--sp = (int)(pc+1); pc = (int *)*pc;}//call subroutine
    //else if (op == RET) {pc = (int *)*sp++;} //return from subroutine

    else if (op == ENT) {*--sp = (int)bp; bp = sp; sp = sp - *pc++;}
    
    else if (op == ADJ) {sp = sp + *pc++;}

    else if (op == LEV) {sp = bp; bp = (int *)*sp++; pc = (int *)*sp++;}

    else if (op == ENT) {*--sp = (int)bp; bp = sp; sp = sp - *pc++;}//make new stack frame
    else if (op == ADJ) {sp = sp + *pc++;}
    else if (op == LEV) {sp = bp; bp = (int *)*sp++; pc = (int *)*sp++;}//restore call frame and PC
    else if (op == LEA) {ax = (int)(bp + *pc++);}
    
    else if (op == OR)  ax = *sp++ | ax;
    else if (op == XOR) ax = *sp++ ^ ax;
    else if (op == AND) ax = *sp++ & ax;
    else if (op == EQ)  ax = *sp++ == ax;
    else if (op == NE)  ax = *sp++ != ax;
    else if (op == LT)  ax = *sp++ < ax;
    else if (op == LE)  ax = *sp++ <= ax;
    else if (op == GT)  ax = *sp++ > ax;
    else if (op == GE)  ax = *sp++ >= ax;
    else if (op == SHL)  ax = *sp++ << ax;
    else if (op == SHR)  ax = *sp++ >> ax;
    else if (op == ADD)  ax = *sp++ + ax;
    else if (op == SUB)  ax = *sp++ - ax;
    else if (op == MUL)  ax = *sp++ * ax;
    else if (op == DIV)  ax = *sp++ / ax;
    else if (op == MOD)  ax = *sp++ % ax;

    else if (op == EXIT) { printf("exit(%d)", *sp); return *sp;}
    else if (op == OPEN) { ax = open((char *)sp[1], sp[0]); }
    else if (op == CLOS) { ax = close(*sp);}
    else if (op == READ) { ax = read(sp[2], (char *)sp[1], *sp); }
    else if (op == PRTF) { tmp = sp + pc[1]; ax = printf((char *)tmp[-1], tmp[-2], tmp[-3], tmp[-4], tmp[-5], tmp[-6]); }
    else if (op == MALC) { ax = (int)malloc(*sp);}
    else if (op == MSET) { ax = (int)memset((char *)sp[2], sp[1], *sp);}
    else if (op == MCMP) { ax = memcmp((char *)sp[2], (char *)sp[1], *sp);}

    else {
      printf("unknown instruction:%d\n", op);
      return -1;
    }
  }
  return 0;
}

int main(int argc, char **argv) {
  int i, fd;
  int *tmp;

  argc--;
  argv++;

  poolsize = 256 * 1024;
  line = 1;

  if ((fd = open(*argv, 0)) < 0) {
    printf("could not open(%s)\n", *argv);
    return -1;
  }

  if (!(text = old_text = malloc(poolsize))) {
    printf("could not malloc(%d) for text area\n", poolsize);
    return -1;
  }

  if (!(data = malloc(poolsize))) {
    printf("could not malloc(%d) for data area\n", poolsize);
    return -1;
  }

  if (!(stack = malloc(poolsize))) {
    printf("could not malloc(%d) for stack area\n", poolsize);
    return -1;
  }

  if (!(symbols = malloc(poolsize))) {
    printf("could not malloc(%d) for symbols table\n", poolsize);
    return -1;
  }

  memset(text, 0, poolsize);
  memset(data, 0, poolsize);
  memset(stack, 0, poolsize);

  bp = sp = (int *)((int)stack + poolsize);
  ax = 0;

  src = "char else enum if int return sizeof while "
        "open read close printf malloc memset memcmp exit void main";

  i = Char;
  while (i <= While) {
    next();
    current_id[Token] = i++;
  }

  i = OPEN;
  while (i <= EXIT) {
    next();
    current_id[Class] = Sys;
    current_id[Type] = INT;
    current_id[Value] = i++;
  }

  next(); current_id[Token] = Char;
  next(); idmain = current_id;

  if ((fd = open(*argv, 0)) < 0) {
    printf("could not open(%s)\n", *argv);
    return -1;
  }

  if (!(src = old_src = malloc(poolsize))) {
    printf("could not malloc(%d)\n", i);
    return -1;
  }

  if ((i = read(fd, src, poolsize-1)) <= 0) {
    printf("read() returned %d\n", i);
    return -1;
  }

  src[i] = 0;
  close(fd);

  program();

  if (!(pc = (int *)idmain[Value])) {
    printf("main() not defined\n");
    return -1;
  }

  //setup stack
  sp = (int*)((int)stack + poolsize);
  *--sp = EXIT;
  *--sp = PUSH; tmp = sp;
  *--sp = argc;
  *--sp = (int)argv;
  *--sp = (int)tmp;

  return eval();
}
