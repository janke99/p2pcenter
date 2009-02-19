/*
 * KAutoPtr.h
 */

#ifndef __KAUTOPTR_H
#define __KAUTOPTR_H

#define RINOK(x) { HRESULT __result_ = (x); if(__result_ != S_OK) return __result_; }

template <class T>
class CKAutoPtr
{
  T* _p;
public:
  // typedef T _PtrClass;
  CKAutoPtr() { _p = NULL;}
  CKAutoPtr(T* p) {if ((_p = p) != NULL) p->AddRef(); }
  CKAutoPtr(const CKAutoPtr<T>& lp)
  {
    if ((_p = lp._p) != NULL)
      _p->AddRef();
  }
  ~CKAutoPtr() { if (_p) _p->Release(); }
  void Release() { if (_p) { _p->Release(); _p = NULL; } }
  operator T*() const {  return (T*)_p;  }
  // T& operator*() const {  return *_p; }
  T** operator&() { return &_p; }
  T* operator->() const { return _p; }
  T* operator=(T* p) 
  { 
    if (p != 0)
      p->AddRef();
    if (_p) 
      _p->Release();
    _p = p;
    return p;
  }
  T* operator=(const CKAutoPtr<T>& lp) { return (*this = lp._p); }
  bool operator!() const { return (_p == NULL); }
  // bool operator==(T* pT) const {  return _p == pT; }
  // Compare two objects for equivalence
  void Attach(T* p2)
  {
    Release();
    _p = p2;
  }
  T* Detach()
  {
    T* pt = _p;
    _p = NULL;
    return pt;
  }
};

//////////////////////////////////////////////////////////

class CKUnknownImp
{
protected:
  ULONG __m_RefCount;
public:
  CKUnknownImp(): __m_RefCount(0) {}
  virtual ~CKUnknownImp(){};

  ULONG AddRef() { return ++__m_RefCount; }
  ULONG Release() { if (--__m_RefCount != 0) return __m_RefCount; delete this; return 0; }
};
#endif


