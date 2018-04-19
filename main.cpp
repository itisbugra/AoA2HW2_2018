#include <iostream>
#include <cstdint>
#include <cmath>
#include <fstream>
#include <vector>
#include <cfloat>
#include <atomic>
#include <chrono>
#include <cstring>

#undef USE_EUCLIDIAN_DIST

typedef uint64_t UInt64;
typedef UInt64 UInt;
typedef int64_t Int64;
typedef Int64 Int;
typedef float Float;

const std::string KWORD_DELIM = " ";

struct Point {
public:
    UInt x;
    UInt y;
    UInt z;
    
    static Float distance_to(const Point &lhs, const Point &rhs) noexcept
    {
        Float x_diff = (Int)lhs.x - (Int)rhs.x;
        Float y_diff = (Int)lhs.y - (Int)rhs.y;
        Float z_diff = (Int)lhs.z - (Int)rhs.z;
        
        return std::sqrt(x_diff * x_diff + y_diff * y_diff + z_diff * z_diff);
    }
};

class Ball {
private:
    const Point _point;
    
public:
    static std::atomic<UInt> NUMBER_OF_COUNTS;
    
    Ball(const Point &point)
    : _point(point)
    {
        //  Blank implementation
    }
    
    Ball()
    : _point(Point())
    {
        
    }
    
    const Float distance_to(const Ball &ball) const noexcept
    {
        //  Increment the static counter
        NUMBER_OF_COUNTS.operator++();
        
        return Point::distance_to(_point, ball._point);
    }
    
    const Point &get_point() const noexcept
    {
        return _point;
    }
};

std::atomic<UInt> Ball::NUMBER_OF_COUNTS{0};

int horizontal_comparator(const void *lhs, const void *rhs)
{
    const Ball *lhs_ball = (const Ball *)lhs;
    const Ball *rhs_ball = (const Ball *)rhs;
    
    return (int)((Int)lhs_ball->get_point().x - (Int)rhs_ball->get_point().x);
}

int vertical_comparator(const void *lhs, const void *rhs)
{
    const Ball *lhs_ball = (const Ball *)lhs;
    const Ball *rhs_ball = (const Ball *)rhs;
    
#ifdef USE_EUCLIDIAN_DIST
    const Float lhs_dist = lhs_ball->get_point().y * lhs_ball->get_point().y + lhs_ball->get_point().z + lhs_ball->get_point().z;
    const Float rhs_dist = rhs_ball->get_point().y * rhs_ball->get_point().y + rhs_ball->get_point().z + rhs_ball->get_point().z;
    
    return (int)((Int)lhs_dist - (Int)rhs_dist);
#else
    return (int)((Int)lhs_ball->get_point().y - (Int)rhs_ball->get_point().y);
#endif
}

Float brute_force(Ball *balls, UInt len)
{
    float minimum_dist = FLT_MAX;
    
    for (UInt a = 0; a < len; ++a) {
        for (UInt b = a + 1; b < len; ++b) {
            Float dist = balls[a].distance_to(balls[b]);
            
            if (dist < minimum_dist) {
                minimum_dist = dist;
            }
        }
    }
    
    return minimum_dist;
}

Float strip_iter(Ball *balls, UInt len, Float dist_maxima)
{
    Float maximum = dist_maxima;
    
    qsort(balls, len, sizeof(Ball), vertical_comparator);
    
    for (UInt a = 0; a < len; ++a) {
        for (UInt b = a + 1; b < len && ((Int)balls[b].get_point().y - (Int)balls[a].get_point().y) < maximum; ++b) {
            Float dist = balls[a].distance_to(balls[b]);
            
            if (dist < maximum) {
                maximum = dist;
            }
        }
    }
    
    return maximum;
}

Float closest_iter(Ball *balls, UInt len)
{
    if (len <= 3) {
        return brute_force(balls, len);
    } else {
        UInt middle = len / 2;
        Ball pivot = balls[middle];
        
        Float dist_lhs = closest_iter(balls, middle);
        Float dist_rhs = closest_iter(balls + middle, len - middle);
        
        Float minima = dist_lhs > dist_rhs ? dist_lhs : dist_rhs;
        
        Ball *closest = new Ball[len]();
        UInt next_idx = 0;
        
        for (UInt i = 0; i < len; ++i) {
            if (std::abs((Int)balls[i].get_point().x - (Int)pivot.get_point().x) < minima) {
                std::memcpy(&closest[next_idx], &balls[i], sizeof(Ball));
                next_idx += 1;
            }
        }
        
        Float strip = strip_iter(closest, next_idx, minima);
        
        return strip > minima ? minima : strip;
    }
}

Float closest_pair_of_points(const std::vector<Ball> &balls)
{
    Ball *arr = new Ball[balls.size()]();
    
    std::memcpy(arr, balls.data(), sizeof(Ball) * balls.size());
    
    qsort(arr, balls.size(), sizeof(Ball), horizontal_comparator);
    
    return closest_iter(arr, balls.size());
}

int main(int argc, const char * argv[]) {
    if (argc != 2) {
        std::cerr << "usage: ./main <file_name>" << std::endl;
        
        exit(9);
    }
    
    std::ifstream input_file(argv[1]);
    
    std::string token;
    std::getline(input_file, token);

    const Int number_of_entries = std::stoi(token);
    
    if (number_of_entries < 0) {
        std::cerr << "error: invalid number of entries" << std::endl;
        
        exit(9);
    }
    
    std::vector<Ball> balls;
    
    for (UInt64 i = 0; i < (UInt64)number_of_entries; ++i) {
        std::getline(input_file, token);
        
        const UInt x = (UInt)std::stoi(token.substr(0, token.find(KWORD_DELIM)));
        
        token = token.substr(token.find(KWORD_DELIM) + 1, token.length());
        
        const UInt y = (UInt)std::stoi(token.substr(0, token.find(KWORD_DELIM)));
        
        token = token.substr(token.find(KWORD_DELIM) + 1, token.length());
        
        const UInt z = (UInt)std::stoi(token.substr(0, token.find(KWORD_DELIM)));
        
        Point point;
        
        point.x = x;
        point.y = y;
        point.z = z;
        
        balls.push_back(Ball(point));
        
        if (input_file.eof()) {
            std::cout << "warning: reached end-of-file before fetching given number of entries" << std::endl;
            
            break;
        }
    }
    
    std::cout << "Number of entries involved in calculation: " << number_of_entries << "." << std::endl;
    
    std::chrono::time_point<std::chrono::high_resolution_clock> start_time = std::chrono::high_resolution_clock::now();
    
    std::cout << "The lowest distance is " << closest_pair_of_points(balls) << "." << std::endl;
    
    std::chrono::time_point<std::chrono::high_resolution_clock> end_time = std::chrono::high_resolution_clock::now();
    
    std::cout << "Number of total distance calculations is " << Ball::NUMBER_OF_COUNTS << "." << std::endl;
    
    std::cout << "Monotonic time elapsed during execution: " << std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time).count() << " ns (1E-9 s)." << std::endl;
    
    return 0;
}





