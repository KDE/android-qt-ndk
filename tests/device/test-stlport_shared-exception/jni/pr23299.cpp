// PR rtl-optimization/23299
// { dg-do run }
// { dg-options "-Os" }

extern "C" void abort ();

struct A
{
  virtual int a () {}
};
struct B : public A
{
  virtual int b () {}
};
struct C : public A
{
  virtual int c () {}
};
struct D
{
  D () { d = 64; }
  ~D ();
  int d;
};

int x;
D::~D ()
{
  x |= 1;
  if (d != 64)
    abort ();
}

struct E : public B, public C
{
  E () {}
  virtual int c ();
  ~E ();
  D dv;
};

E::~E ()
{
  int r = c ();
}

int
E::c ()
{
  if (x > 10)
    throw 1;
  x |= 2;
}

int
main (void)
{
  {
    E e;
  }
  if (x != 3)
    abort ();
}
