//
// Created by Matrix on 2024/6/18.
//

export module UI.Creation.Dialog;

import std;

export import UI.Dialog;
export import UI.FileTreeSelector;

import UI.Creation;

export namespace UI::Create{

	template <Concepts::Derived<FileSelector> FT, std::invocable<FT&> Init>
	struct FileDialog{
		Init func{};

		[[nodiscard]] explicit FileDialog(Init init) : func{init}{}

		void operator ()(Dialog& dialog) const{
			static_cast<UI::LayoutCell&>(dialog.content.add<FT>([this, &dialog](FT& selector){
				func(selector);

				selector.gotoDirectory(OS::File{std::filesystem::current_path().parent_path()}, false);
				selector.createDialogQuitButton(dialog);
			})).fillParent().setAlign(Align::Layout::top_center);
		}
	};

	template <typename Func>
	using BasicFileDialog = FileDialog<FileSelector, Func>;

	template <typename Func>
	using OutputFileDialog = FileDialog<OutputFileSelector, Func>;
}
