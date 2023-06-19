#include <wasm/function_inst.hpp>

namespace wasm {

FunctionInstBase::FunctionInstBase(wasm_function_inst_t function)
    : m_function{function} {}

FunctionInstBase::FunctionInstBase(FunctionInstBase &&other)
    : FunctionInstBase{} {
  swap(*this, other);
}

FunctionInstBase &FunctionInstBase::operator=(FunctionInstBase &&other) {
  swap(*this, other);
  return *this;
}

void FunctionInstBase::swap(FunctionInstBase &a, FunctionInstBase &b) {
  using std::swap;

  swap(a.m_function, b.m_function);
}

wasm_function_inst_t FunctionInstBase::native_handle() const {
  return m_function;
}

} // namespace wasm
