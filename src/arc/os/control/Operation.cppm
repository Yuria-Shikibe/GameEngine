//
// Created by Matrix on 2024/5/10.
//

export module OS.Ctrl.Operation;

import std;
export import OS.Ctrl.Bind;
export import OS.Ctrl.Bind.Constants;
import ext.Heterogeneous;

export namespace Ctrl{

	/**
	 * @brief Localized info
	 */
	struct Instruction{
		static constexpr std::string_view Unavaliable = "N/A";

		std::string name{Unavaliable};
		std::string desc{Unavaliable};
	};

	struct Operation{
		/**
		 * @brief This Should Be UNIQUE if in a same group, and SSO is better
		 */
		std::string name{};
		OS::InputBind defaultBind{};
		OS::InputBind customeBind{};

		[[nodiscard]] Operation() = default;

		[[nodiscard]] Operation(const std::string& name, const OS::InputBind& bind)
			: name{name},
			  defaultBind{bind},
			  customeBind{bind}{}

		[[nodiscard]] Operation(const std::string& name, OS::InputBind&& bind)
			: name{name},
			  defaultBind{std::move(bind)},
			  customeBind{defaultBind}{}

		Instruction instruction{};

		void setDef(){
			customeBind = defaultBind;
		}

		friend bool operator==(const Operation& lhs, const Operation& rhs){
			return lhs.name == rhs.name;
		}

		friend bool operator!=(const Operation& lhs, const Operation& rhs){ return !(lhs == rhs); }
	};

	class OperationGroup{
		std::string name{};
		ext::StringMap<Operation> keyBinds{};
		ext::StringMap<Operation> mouseBinds{};

		std::unordered_map<int, unsigned> groupOccupiedKeys{};

		/**
		 * @return false if conflicted
		 */
		bool registerBindCount(const Operation& operation){
			const auto key = operation.customeBind.getFullKey();
			if(const auto itr = groupOccupiedKeys.find(key); itr != groupOccupiedKeys.end()){
				itr->second++;
				if(itr->second > 1)return false;
			}else{
				groupOccupiedKeys.insert_or_assign(key, 1);
			}

			return true;
		}
	public:
		[[nodiscard]] const std::string& getName() const noexcept{ return name; }

		bool isConfilcted(const int key){
			if(const auto itr = groupOccupiedKeys.find(key); itr != groupOccupiedKeys.end()){
				return itr->second <= 1;
			}
			return true;
		}

		void addKey(const Operation& operation){
			registerBindCount(operation);
			if(Ctrl::onMouse(operation.customeBind.getKey())){
				mouseBinds.insert_or_assign(operation.name, operation);
			}else{
				keyBinds.insert_or_assign(operation.name, operation);
			}
		}

		void addKey(Operation&& operation){
			registerBindCount(operation);
			auto _name = operation.name;

			if(Ctrl::onMouse(operation.customeBind.getKey())){
				mouseBinds.insert_or_assign(std::move(_name), std::move(operation));
			}else{
				keyBinds.insert_or_assign(std::move(_name), std::move(operation));
			}
		}
	};

}

export
template<>
struct ::std::equal_to<Ctrl::Operation>{
	bool operator()(const Ctrl::Operation& l, const Ctrl::Operation& r) const noexcept{
		return l.name == r.name;
	}
};

export
template<>
struct ::std::equal_to<Ctrl::OperationGroup>{
	bool operator()(const Ctrl::OperationGroup& l, const Ctrl::OperationGroup& r) const noexcept{
		return l.getName() == r.getName();
	}
};

