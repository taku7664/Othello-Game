
enum class ColorType
{
	None,
	Black,
	White,
	Count,
};

inline const char* ColorTypeToString(ColorType color)
{
	switch ( color )
	{
	case ColorType::None:
		return "무색";
		break;
	case ColorType::Black:
		return "검정색";
		break;
	case ColorType::White:
		return "하얀색";
		break;
	default:
		return "Unknown";
		break;
	}
}

inline ImVec4 ColorTypeToImVec4(ColorType color)
{
	switch ( color )
	{
	case ColorType::None:
		return ImVec4( 0 , 0 , 0 , 0 );
		break;
	case ColorType::Black:
		return ImVec4( 0 , 0 , 0 , 1 );
		break;
	case ColorType::White:
		return ImVec4( 1 , 1 , 1 , 1 );
		break;
	default:
		return ImVec4( 0 , 0 , 0 , 0 );
		break;
	}
}