//
// Created by Matrix on 2024/3/31.
//

export module UI.InputArea;

export import UI.Elem;

import std;
import Font.GlyphArrangement;

export namespace UI{
	class InputArea : public UI::Elem{
	protected:
		Font::TextString inputContent;


	public:
		Font::TextString getTextCopy(){
			return inputContent;
		}

		Font::TextView getTextView(){
			return inputContent;
		}
	};
}
