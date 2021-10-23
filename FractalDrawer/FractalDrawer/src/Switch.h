#pragma once

template<typename T>
struct Switch {
public:
    T& v1, &v2;

    constexpr Switch(T& v1_, T& v2_, bool isFirst_ = true) : v1{ v1_ }, v2{ v2_ }, m_isFirst{ isFirst_ } {}

    constexpr T& get() {
        if (m_isFirst) return v1;
        else return v2;
    }

    constexpr T& getOther() {
        if (m_isFirst) return v2;
        else return v1;
    }

    constexpr void doSwitch() {
        m_isFirst = !m_isFirst;
    }

    constexpr bool isFirst() {
        return m_isFirst;
    }
private:
    bool m_isFirst;
};