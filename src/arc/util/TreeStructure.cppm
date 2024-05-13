//
// Created by Matrix on 2024/4/13.
//

export module ext.TreeStructure;

import std;
import ext.Concepts;

export namespace ext{
	template <typename T, typename RepresentT = T*, typename KeyEqual = std::equal_to<RepresentT>>
	class TreeStructure{
	public:
		using ValueType = T;
		using RepresentType = RepresentT;
		using KeyEqualer = KeyEqual;

		RepresentType defRootRepresent{};
		static constexpr KeyEqualer equaler{};


		struct TreeNode{
			RepresentType representor{};
			TreeNode* parent{nullptr};
			T* value{nullptr};

			//TODO uses set instead?
			std::list<TreeNode> children{};

			[[nodiscard]] bool hasChildern() const{
				return !children.empty();
			}

			[[nodiscard]] bool hasParent() const{
				return parent != nullptr;
			}

			[[nodiscard]] bool hasValue() const{
				return value != nullptr;
			}

			friend bool operator==(const TreeNode& lhs, const TreeNode& rhs){
				return &lhs == &rhs;
			}

			friend bool operator!=(const TreeNode& lhs, const TreeNode& rhs){
				return &lhs != &rhs;
			}

			void printIt(std::ostream& stream, const int depth = 0, const std::string_view treePrefix = "| ") const{
				for(int i = 0; i < depth; ++i){
					stream << treePrefix;
				}

				if(value){
					stream << representor << " - " << *value;
				} else{
					stream << representor;
				}

				if(!children.empty()){
					stream << '>';
				}

				stream << '\n';

				for(const auto& child : children){
					child.printIt(stream, depth + 1, treePrefix);
				}

			}
		};

	protected:
		friend std::ostream& operator<<(std::ostream& os, const TreeStructure& obj){
			obj.root.printIt(os);
			return os;
		}

		std::vector<T> rawData{};

		template <Concepts::Iterable<RepresentType> ParentRange>
		std::pair<TreeNode*, decltype(std::declval<ParentRange>().begin())>
			findLastMatchNode(ParentRange&& elemParents){
			auto currentDepth = elemParents.begin();

			TreeNode* lastMatchedNode{&root};

			while(currentDepth != elemParents.end()){
				const RepresentType& current = currentDepth.operator*();

				for(TreeNode& child : lastMatchedNode->children){
					if(equaler.operator()(child.representor, current)){
						currentDepth.operator++();
						lastMatchedNode = &child;
						goto loopContinue;
					}
				}

				break;
				loopContinue:{}
			}

			return std::make_pair(lastMatchedNode, currentDepth);
		}


		template <Concepts::Iterable<RepresentType> ParentRange, Concepts::Invokable<ParentRange(const T&)> Func>
		void build(Func&& insertPred){
			initTree();

			for(auto&& element : rawData){
				this->template insert<false, ParentRange, Func>(element, std::forward<Func>(insertPred));
			}
		}


		template <bool addToRaw = true, Concepts::Iterable<RepresentType> ParentRange, Concepts::Invokable<ParentRange
			(const T&)> Func>
		void insert(auto&& element, Func&& insertPred) requires std::convertible_to<decltype(element), T>{
			ParentRange&& elemParents = insertPred(element);

			auto [lastMatchedNode, currentDepth] = this->findLastMatchNode(elemParents);

			for(auto& representor : std::ranges::subrange{currentDepth, elemParents.end()}){
				lastMatchedNode = &lastMatchedNode->children.emplace_back(std::move(representor), lastMatchedNode);
			}

			if constexpr(addToRaw){
				rawData.push_back(std::forward<T>(element));
				lastMatchedNode->value = &rawData.back();
			} else{
				lastMatchedNode->value = &element;
			}
		}


		template <Concepts::Iterable<RepresentType> ParentRange, Concepts::Invokable<ParentRange(const T&)> Func>
		bool erase(const T& element, Func&& insertPred){
			if(std::erase(rawData, element)){
				ParentRange&& elemParents = insertPred(element);

				auto [lastMatchedNode, currentDepth] = this->findLastMatchNode(elemParents);

				if(currentDepth != elemParents.end()){
					return false;
				}

				return static_cast<bool>(std::erase(lastMatchedNode->parent->children, *lastMatchedNode));
			}
			return false;
		}

		template <Concepts::Iterable<RepresentType> ParentRange, Concepts::Invokable<ParentRange(const T&)> Func>
		TreeNode* find(const T& element, Func&& insertPred){
			ParentRange&& elemParents = insertPred(element);

			auto [lastMatchedNode, currentDepth] = this->findLastMatchNode(elemParents);

			if(currentDepth != elemParents.end()){
				return nullptr;
			}

			return lastMatchedNode;
		}

	public:
		[[nodiscard]] std::vector<T>& getRawData(){ return rawData; }

		void clearData(){
			rawData.clear();
			initTree();
		}

		void initTree(){
			root.children.clear();
		}

		TreeNode root{defRootRepresent};

		template <Concepts::InvokableVoid<void(const T&)> Func>
		void build(Func&& insertPred){
			this->template build<std::invoke_result_t<Func, const T&>>(std::forward<Func>(insertPred));
		}

		template <bool addToRaw = true, Concepts::InvokableVoid<void(const T&)> Func>
		void insert(auto&& element, Func&& insertPred) requires std::convertible_to<decltype(element), T>{
			this->template insert<addToRaw, std::invoke_result_t<Func, const T&>>(
				std::forward<T>(element), std::forward<Func>(insertPred));
		}

		template <Concepts::InvokableVoid<void(const T&)> Func>
		void erase(const T& element, Func&& insertPred){
			this->template erase<std::invoke_result_t<Func, const T&>>(element, std::forward<Func>(insertPred));
		}

		template <Concepts::InvokableVoid<void(const T&)> Func>
		TreeNode* find(const T& element, Func&& insertPred){
			return this->template find<std::invoke_result_t<Func, const T&>>(element, std::forward<Func>(insertPred));
		}
	};
}
