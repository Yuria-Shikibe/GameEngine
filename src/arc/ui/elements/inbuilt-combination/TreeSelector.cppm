//
// Created by Matrix on 2024/3/31.
//

export module UI.TreeSelector;

export import UI.Table;
export import UI.Button;
export import UI.ScrollPane;
export import UI.Label;
export import ext.TreeStructure;
import ext.Concepts;

export namespace UI{
	template <typename T, typename RepresentType = T*>
	class TreeSelector : public Table{
	public:
		TreeSelector(){
			defaultCellLayout.fillParentX().wrapY().setMargin(5);
			setCellAlignMode(Align::Layout::top_center);
		}

		ext::TreeStructure<T, RepresentType> tree{};
		// ext::TreeStructure<int, int> tree{};
		using TreeType = decltype(tree);

		using NodeType = typename TreeType::TreeNode;

		NodeType* currentParent{&tree.root};
		int lastIndex{-1};

		template <Concepts::InvokableVoid<void(const T&)> Func>
		void build(Func&& func){
			tree.build(func);
			buildCurrentPage();
		}

		[[nodiscard]] bool hasSelect() const{
			return lastIndex >= 0 && lastIndex < currentParent->children.size();
		}

		void freeSelect(){
			lastIndex = -1;
		}

		void sortCurrentPageWith(){
			clearChildrenSafely();

			buildCurrentPage();
		}

		//TODO uses template derivation maybe
		virtual void buildSingle(UI::Table& table, NodeType& node) = 0;

		virtual void buildRetrun(UI::Button& button, NodeType& node) = 0;

		void returnToParentDirectory(){
			currentParent = currentParent->parent;
			buildCurrentPage();
		}

		void gotoChildDirectory(NodeType& node){
			currentParent = &node;
			buildCurrentPage();
		}

		void buildCurrentPage(){
			clearChildrenSafely();

			if(currentParent->parent){
				Table::add<UI::Button>([this](UI::Button& button){
					this->buildRetrun(button, *currentParent->parent);
				}).wrapY().endLine().setAlign(Align::Layout::top_center);
			}

			Table::add<UI::ScrollPane>([this](UI::ScrollPane& pane){
				pane.setItem<UI::Table>([this](UI::Table& table){
					table.setEmptyDrawer();
					table.setFillparentX();
					table.defaultCellLayout.fillParentX().wrapY();

					for(auto& child : currentParent->children){
						this->buildSingle(table, child);
					}
				});
			}).fillParentY();

			layout();
		}
	};
}
