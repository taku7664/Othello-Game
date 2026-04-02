
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
		return "None";
		break;
	case ColorType::Black:
		return "Black";
		break;
	case ColorType::White:
		return "White";
		break;
	default:
		return "Unknown";
		break;
	}
}