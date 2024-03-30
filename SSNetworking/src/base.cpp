#include <SSNetworking/base.hpp>
#include <boost/lambda/lambda.hpp>
#include <iostream>
#include <iterator>
#include <algorithm>


void MultiplyByThree() {
    std::for_each(std::istream_iterator<int>(std::cin), std::istream_iterator<int>(), std::cout << (boost::lambda::_1 * 3) << " ");
}