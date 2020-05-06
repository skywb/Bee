#ifndef ERROR_H_ILGNPXK2
#define ERROR_H_ILGNPXK2

#include <string>
#include <functional>

namespace Bee {
	class Error
	{
		public:
			enum Type {
				kSusseed = 0,
				kClientEmpty = 1,
				kRemoteRused = 2,
				kLocalAddressError = 3
			};
			Error (Type type) : type_(type) {}
			virtual ~Error () {}
			const std::string Message() {
				switch (type_) {
					case kSusseed :
						return std::string("susseed");	
						break;
					case kClientEmpty :
						return std::string("Not found a object to send");	
						break;
					case kRemoteRused :
						return std::string("No connection could be made because the target machine actively refused it");	
						break;
					case kLocalAddressError :
						return std::string("Local address is can't used");	
						break;
					default:
						return std::string("Not Found Error Type");

				}
			}
		operator bool() const {
			return type_ != kSusseed;
		}
		Type GetType() { return type_; }
		private:
			Type type_;
	};


	typedef std::function<void(const Error)> BeeCallback;
}

#endif /* end of include guard: ERROR_H_ILGNPXK2 */
