#include "udf_holder.h"

int main() {
    {
        using TestVoidCallHolder = UDFHolder<TestVoidCall, VectorExec, std::string, int32_t, bool>;
        std::cout << TestVoidCallHolder::udf_has_call_return_bool << std::endl;
        std::string out;
        TestVoidCallHolder().callImpl(out, 21, false);
    }

    {
        using TestBoolCallHolder = UDFHolder<TestBoolCall, VectorExec, int32_t>;
        std::cout << TestBoolCallHolder::udf_has_call_return_bool << std::endl;
        int32_t out;
        TestBoolCallHolder().callImpl(out);
    }
    return 0;
}