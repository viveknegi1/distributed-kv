#include <iostream>

int main(int argc, char* argv[])
{

     if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <your_argument>" << std::endl;
        return 1; 
    }
    std::cout << "Your node id is: " << argv[1] << std::endl;
    
    return 0 ;

}