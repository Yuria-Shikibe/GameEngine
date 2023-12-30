module OS;

import <sstream>;
import File;
import StackTrace;
import Core;
import Assets.Graphic;

void OS::exitApplication(const int s, const std::string_view what) {
	std::stringstream ss;

	if(!what.empty()) {
		ss << what << "\n\n";
	}

	ss << "Crashed! : Code-" << s << '\n';

	ext::getStackTraceBrief(ss);

	const OS::File file{OS::crashFileGetter()};

	file.writeString(ss.str());

	//TODO exception resource release
	Assets::dispose();
	Core::dispose();

	std::exit(s);
}
