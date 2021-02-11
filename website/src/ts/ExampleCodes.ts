export const kinds = `#include <mpp/matrix.h>

int main()
{
    auto m_fully_static = mpp::matrix<int, 3, 3>{};
    auto m_fully_dynamic = mpp::matrix<int, std::dynamic_extent, std::dynamic_extent>{};
    // ^ or just mpp::matrix<int>{} since the size parameters are defaulted to dynamic extent
    auto m_dynamic_rows = mpp::matrix<int, std::dynamic_Extent, 3>{};
    auto m_dynamic_columns = mpp::matrix<int, 3, std::dynamic_extent>{};
    // ^ or just mpp::matrix<int, 3>{} since the column parameter is defaulted to dynamic extent

    return 0;
}`;

export const operations = `#include <mpp/matrix.h>
#include <mpp/operations.hpp> // +, -, *, and /
#include <mpp/algorithms.hpp> // determinant, inverse, transpose, block, and etc...
#include <utility> // std::type_identity

int main()
{
    auto m = mpp::matrix{
        { 1, 2, 3 },
        { 1, 2, 3 },
        { 1, 2, 3 }
    }; // Uses 2D initializer list initializer. Defaults to dynamic matrix

    // Remember that math operations are expression templates, so the results are not evaluated immediately
    auto expr = m + m - m * 2 + m / 3;

    auto result = expr(0, 0); // This only evaluates the result at (1, 1)
    auto result_m = mpp::matrix{ expr }; // Evaluates the whole expression and constructs a matrix

    auto m_det = mpp::determinant(m); // You can specify larger types for safety to avoid type overflow from calculation

    auto m_inv = mpp::inverse(std::type_identity<float>{}, m); // Use "float" for result matrix type
    auto m_inv_2 = mpp::inverse(std::type_identity<int>{}, m); // Compile error: user provided type must be a floating point!
    auto m_inv_dbl = mpp::inverse(m); // Defaults to "double" for result matrix type

    auto m_transposed = mpp::transpose(m);
    auto m_block = mpp::block(m, 0, 0, 1, 1); // Grabs top corner 2 x 2 (the indexes are inclusive, so 1 x 1
                                              // blocks are possible :) )

    return 0;
}`;

export const customize_default_extent = `// Import appropriate tags for customization
#include <mpp/utility/config.hpp>

namespace mpp::customize
{
    /**
     * Customizations has to take place BEFORE ANY INSTANTIATIONS!
     * (it will NOT be detected in time after).
     * 
     * The namespace mpp::customize is where the user can freely open to customize.
     */

    [[nodiscard]] constexpr std::size_t tag_invoke(matrix_rows_extent_tag, customize_tag)
    {
        return 10;
    }

    [[nodiscard]] constexpr std::size_t tag_invoke(matrix_columns_extent_tag, customize_tag)
    {
        return 10;
    }
} // namespace mpp::customize

#include <mpp/matrix.hpp>

int main()
{
    auto m = mpp::matrix<int>{}; // The library uses the new default extent by default
    auto r = m.rows_extent(); // 10
    auto c = m.columns_extent(); // 10

    return 0;
}
`;

export const customize_utilities = `#include <mpp/matrix.hpp>

namespace ns
{
    enum class vec_types
    {
        vec
    };

    struct vec {};

    [[nodiscard]] auto tag_invoke(mpp::type_t, vec) -> vec_types
    {
        return vec_types::vec;
    }

    [[nodiscard]] auto tag_invoke(mpp::determinant_t, vec) -> int
    {
        return 2000;
    }

    // You can customize even more customization points than shown here, but
    // this is just to demonstrate the library picking up the customizations
}

int main()
{
    auto v = ns::vec{};
    auto t = mpp::type(v); // vec_types::vec
    auto d = mpp::determinant(v); // 2000

    return 0;
}
`;
