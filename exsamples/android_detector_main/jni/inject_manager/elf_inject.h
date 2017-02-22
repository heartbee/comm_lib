
class ElfInject{
public:
	ElfInject();
	~ElfInject();

	bool Inject(std::string &target_lib_name, std::string& inject_lib_name, std::string& start_func);
};
