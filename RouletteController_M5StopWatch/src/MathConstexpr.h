#ifndef MATH_CONSTEXPR_H
#define MATH_CONSTEXPR_H

#include <cmath>

// constexpr 版の mod 関数
template <typename T>
constexpr T mod_constexpr(T x, T y)
{
    return x - y * static_cast<T>(static_cast<long long>(x / y));
}

// 角度を 0 から 2π の範囲に正規化する関数
template <typename T>
constexpr T standardize_angle_constexpr(T theta)
{
    const T twoPi = static_cast<T>(M_PI) * static_cast<T>(2);
    theta = mod_constexpr(theta, twoPi);
    if (theta < 0.f)
    {
        theta += twoPi;
    }
    return theta;
}

// constexpr 版の sin の部分近似関数（0 < theta < pi/4 くらいで正確な近似）
template <typename T>
constexpr T sin_partial_constexpr(T theta)
{
    // マクローリン展開による近似
    return theta - (theta * theta * theta) / static_cast<T>(6) +
           (theta * theta * theta * theta * theta) / static_cast<T>(120);
}

// constexpr 版の cos の部分近似関数（0 < theta < pi/4 くらいで正確な近似）
template <typename T>
constexpr T cos_partial_constexpr(T theta)
{
    // マクローリン展開による近似
    return static_cast<T>(1) - (theta * theta) / static_cast<T>(2) +
           (theta * theta * theta * theta) / static_cast<T>(24);
}

// constexpr 版の sin 関数
template <typename T>
constexpr T sin_constexpr(T theta)
{
    theta = standardize_angle_constexpr(theta);
    const T pi = static_cast<T>(M_PI);
    const T halfPi = pi / static_cast<T>(2);
    const T quarterPi = pi / static_cast<T>(4);

    // 角度に応じて正しい象限の三角関数近似を選択する
    if (theta < quarterPi)
    {
        return sin_partial_constexpr(theta);
    }
    else if (theta < halfPi)
    {
        return cos_partial_constexpr(halfPi - theta);
    }
    else if (theta < pi * static_cast<T>(3) / static_cast<T>(4))
    {
        return cos_partial_constexpr(theta - halfPi);
    }
    else if (theta < pi)
    {
        return sin_partial_constexpr(pi - theta);
    }
    else if (theta < pi * static_cast<T>(5) / static_cast<T>(4))
    {
        return -sin_partial_constexpr(theta - pi);
    }
    else if (theta < pi * static_cast<T>(3) / static_cast<T>(2))
    {
        return -cos_partial_constexpr(pi * static_cast<T>(3) / static_cast<T>(2) - theta);
    }
    else if (theta < pi * static_cast<T>(7) / static_cast<T>(4))
    {
        return -cos_partial_constexpr(theta - pi * static_cast<T>(3) / static_cast<T>(2));
    }
    else
    {
        return -sin_partial_constexpr(pi * static_cast<T>(2) - theta);
    }
}

// constexpr 版の cos 関数
template <typename T>
constexpr T cos_constexpr(T theta)
{
    theta = standardize_angle_constexpr(theta);
    const T pi = static_cast<T>(M_PI);
    const T halfPi = pi / static_cast<T>(2);
    const T quarterPi = pi / static_cast<T>(4);

    // 角度に応じて正しい象限の三角関数近似を選択する
    if (theta < quarterPi)
    {
        return cos_partial_constexpr(theta);
    }
    else if (theta < halfPi)
    {
        return sin_partial_constexpr(halfPi - theta);
    }
    else if (theta < pi * static_cast<T>(3) / static_cast<T>(4))
    {
        return -sin_partial_constexpr(theta - halfPi);
    }
    else if (theta < pi)
    {
        return -cos_partial_constexpr(pi - theta);
    }
    else if (theta < pi * static_cast<T>(5) / static_cast<T>(4))
    {
        return -cos_partial_constexpr(theta - pi);
    }
    else if (theta < pi * static_cast<T>(3) / static_cast<T>(2))
    {
        return -sin_partial_constexpr(theta - pi);
    }
    else if (theta < pi * static_cast<T>(7) / static_cast<T>(4))
    {
        return sin_partial_constexpr(theta - pi * static_cast<T>(3) / static_cast<T>(2));
    }
    else
    {
        return cos_partial_constexpr(pi * static_cast<T>(2) - theta);
    }
}

#endif // MATH_CONSTEXPR_H
