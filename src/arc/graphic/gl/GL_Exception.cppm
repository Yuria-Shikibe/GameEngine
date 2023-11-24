export module GL.GL_Exception;

import <exception>;

export class GL_Exception final : public std::exception{
public:
	GL_Exception() = default;

	explicit GL_Exception(char const* message) : std::exception(message){

	}

	GL_Exception(char const* message, const int i) : std::exception(message, i){

	}

	explicit GL_Exception(exception const& other): exception(other){
	}
};

