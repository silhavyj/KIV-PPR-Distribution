#pragma once

#include <mutex>
#include <memory>

namespace kiv_ppr
{
    /// \author Jakub Silhavy
    /// @tparam T Class to be used as a singleton
    ///
    /// This class works as a wrapper for any class that
    /// is intended to be used as a singleton. The class
    /// must provide a default constructor.
    template<typename T>
    class Singleton
    {
    public:
        /// Returns the instance of the class. 
        /// @return Instance of the class 
        static T* Get_Instance()
        {
            // Instantiate the class only once.
            std::call_once(s_init_flag, [=]() {
                Singleton<T>::s_instance = std::make_unique<T>();
            });

            // Return the instance.
            return s_instance.get();
        }

    private:
        static std::unique_ptr<T> s_instance; ///< Instance of the class (typename T)
        static std::once_flag s_init_flag;    ///< Flag indicating whether or not the class has been instantiated
    };

    // Initialize static class members.

    template<typename T>
    std::unique_ptr<T> Singleton<T>::s_instance = nullptr;

    template<typename T>
    std::once_flag Singleton<T>::s_init_flag = {};
}

// EOF