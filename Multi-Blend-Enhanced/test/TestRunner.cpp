#include "PluginTestSuite.h"
#include <iostream>
#include <chrono>

int main()
{
    std::cout << "🚀 Multi-Blend Enhanced Plugin Test Runner" << std::endl;
    std::cout << "==========================================" << std::endl;
    std::cout << "Testing for Logic Pro compatibility and stability..." << std::endl;
    std::cout << std::endl;
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    try
    {
        PluginTestSuite testSuite;
        bool allTestsPassed = testSuite.runAllTests();
        
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(endTime - startTime);
        
        std::cout << std::endl;
        std::cout << "⏱️  Total test duration: " << duration.count() << " seconds" << std::endl;
        
        if (allTestsPassed)
        {
            std::cout << std::endl;
            std::cout << "🎉 SUCCESS: All tests passed!" << std::endl;
            std::cout << "✅ Plugin is ready for Logic Pro deployment" << std::endl;
            std::cout << "✅ No crashes detected" << std::endl;
            std::cout << "✅ Memory management verified" << std::endl;
            std::cout << "✅ Thread safety confirmed" << std::endl;
            std::cout << "✅ Performance within acceptable limits" << std::endl;
            return 0;
        }
        else
        {
            std::cout << std::endl;
            std::cout << "❌ FAILURE: Some tests failed!" << std::endl;
            std::cout << "⚠️  Plugin needs fixes before Logic Pro deployment" << std::endl;
            std::cout << "⚠️  Please review failed tests above" << std::endl;
            return 1;
        }
    }
    catch (const std::exception& e)
    {
        std::cout << std::endl;
        std::cout << "💥 CRITICAL ERROR: Test suite crashed!" << std::endl;
        std::cout << "Error: " << e.what() << std::endl;
        std::cout << "❌ Plugin is NOT ready for Logic Pro deployment" << std::endl;
        return 1;
    }
    catch (...)
    {
        std::cout << std::endl;
        std::cout << "💥 CRITICAL ERROR: Unknown exception in test suite!" << std::endl;
        std::cout << "❌ Plugin is NOT ready for Logic Pro deployment" << std::endl;
        return 1;
    }
}