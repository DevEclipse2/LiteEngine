#pragma once
namespace NodeEditor
{
    // generic Data Tree for stuff
        // hey did you know basically everything is some spin off of an int or a bool or a float
        //and those are remixes of the one and only [has electrons flowing through it]
    //this is pretty useful
    struct VariantNode;
    using NodeValue = std::variant<int, double, std::string, bool, std::vector<VariantNode>>;

    struct VariantNode {
        NodeValue data;
    };
	class Node
	{
	public:

		//inline static 


	};

}