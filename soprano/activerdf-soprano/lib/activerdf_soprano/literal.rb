require 'active_rdf'

module QtLiteral
  Namespace.register :xsd, 'http://www.w3.org/2001/XMLSchema#'
  def xsd_type
    case self
    when Qt::ByteArray
      XSD::base64Binary
    when Qt::DateTime, Qt::Date, Qt::Time
      XSD::date
    end
  end

  def self.typed(value, type)
    case type
    when XSD::base64Binary
      Qt::ByteArray.new(value.to_s)
    when XSD::date
      Qt::DateTime.parse(value)
    end
  end

  def to_ntriple
    if $activerdf_without_xsdtype
      "\"#{to_s}\""
    else
      "\"#{to_s}\"^^#{xsd_type}"
    end
  end
end

class Qt::ByteArray; include QtLiteral; end
class Qt::DateTime; include QtLiteral; end
class Qt::Date; include QtLiteral; end
class Qt::Time; include QtLiteral; end

