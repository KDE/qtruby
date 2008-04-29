// -*- Mode: C++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
/*
    This file is part of KDE.

    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2003 Waldo Bastian <bastian@kde.org>
    Copyright (c) 2003 Zack Rusin <zack@kde.org>
    Copyright (c) 2006 Michaël Larouche <michael.larouche@kdemail.net>
    Copyright (c) 2008 Richard Dale <richard.j.dale@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <QtCore/QCoreApplication>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QSettings>
#include <QtCore/QTextStream>
#include <QtXml/QDomAttr>
#include <QtCore/QRegExp>
#include <QtCore/QStringList>

#include <ostream>
#include <iostream>
#include <stdlib.h>


static inline std::ostream &operator<<(std::ostream &o, const QString &str)
{
    o << str.toLocal8Bit().constData();
    return o;
}

static void parseArgs(const QStringList &args, QString &directory, QString &file1, QString &file2)
{
    int fileCount = 0;
    directory = ".";

    for (int i = 1; i < args.count(); ++i) {
        if (args.at(i) == "-d" ||  args.at(i) == "--directory") {
            if (i + 1 > args.count()) {
                std::cerr << qPrintable(args.at(i)) << " needs an argument" << std::endl;
                exit(1);
            }
            directory = args.at(++i);
        } else if (args.at(i).startsWith("-d")) {
            directory = args.at(i).mid(2);
        } else if (args.at(i) == "--help" || args.at(i) == "-h") {
            std::cout << "Options:" << std::endl;
            std::cout << "  -L --license              Display software license" << std::endl;
            std::cout << "  -d, --directory <dir>     Directory to generate files in [.]" << std::endl;
            std::cout << "  -h, --help                Display this help" << std::endl;
            std::cout << std::endl;
            std::cout << "Arguments:" << std::endl;
            std::cout << "      file.kcfg                 Input kcfg XML file" << std::endl;
            std::cout << "      file.kcfgc                Code generation options file" << std::endl;
            exit(0);
        } else if (args.at(i) == "--license" || args.at(i) == "-L") {
            std::cout << "Copyright 2003 Cornelius Schumacher, Waldo Bastian, Zack Rusin," << std::endl;
            std::cout << "    Reinhold Kainhofer, Duncan Mac-Vicar P., Harald Fernengel" << std::endl;
            std::cout << "    Richard Dale" << std::endl;
            std::cout << "This program comes with ABSOLUTELY NO WARRANTY." << std::endl;
            std::cout << "You may redistribute copies of this program" << std::endl;
            std::cout << "under the terms of the GNU Library Public License." << std::endl;
            std::cout << "For more information about these matters, see the file named COPYING." << std::endl;
            exit(0);
        } else if (args.at(i).startsWith('-')) {
            std::cerr << "Unknown option: " << qPrintable(args.at(i)) << std::endl;
            exit(1);
        } else if (fileCount == 0) {
            file1 = args.at(i);
            ++fileCount;
        } else if (fileCount == 1) {
            file2 = args.at(i);
            ++fileCount;
        } else {
            std::cerr << "Too many arguments" << std::endl;
            exit(1);
        }
    }
    if (fileCount < 2) {
        std::cerr << "Too few arguments" << std::endl;
        exit(1);
    }
}

bool globalEnums;
bool useEnumTypes;
bool itemAccessors;
QStringList allNames;
QRegExp *validNameRegexp;
QString This;
QString Const;

struct SignalArguments
{
      QString type;
      QString variableName;
};

class Signal {
public:
  QString name;
  QString label;
  QList<SignalArguments> arguments;
};

class CfgEntry
{
  public:
    struct Choice
    {
      QString name;
      QString context;
      QString label;
      QString whatsThis;
    };
    class Choices
    {
      public:
        Choices() {}
        Choices( const QList<Choice> &d, const QString &n, const QString &p )
             : prefix(p), choices(d), mName(n)
        {
          int i = n.indexOf("::");
          if (i >= 0)
            mExternalQual = n.left(i + 2);
        }
    QString prefix;
        QList<Choice> choices;
    const QString& name() const  { return mName; }
    const QString& externalQualifier() const  { return mExternalQual; }
    bool external() const  { return !mExternalQual.isEmpty(); }
      private:
        QString mName;
        QString mExternalQual;
    };

    CfgEntry( const QString &group, const QString &type, const QString &key,
              const QString &name, const QString &context, const QString &label,
              const QString &whatsThis, const QString &code,
              const QString &defaultValue, const Choices &choices, const QList<Signal> signalList,
              bool hidden )
      : mGroup( group ), mType( type ), mKey( key ), mName( name ),
        mContext( context ), mLabel( label ), mWhatsThis( whatsThis ),
        mCode( code ), mDefaultValue( defaultValue ), mChoices( choices ),
        mSignalList(signalList), mHidden( hidden )
    {
    }

    void setGroup( const QString &group ) { mGroup = group; }
    QString group() const { return mGroup; }

    void setType( const QString &type ) { mType = type; }
    QString type() const { return mType; }

    void setKey( const QString &key ) { mKey = key; }
    QString key() const { return mKey; }

    void setName( const QString &name ) { mName = name; }
    QString name() const { return mName; }

    void setContext( const QString &context ) { mContext = context; }
    QString context() const { return mContext; }

    void setLabel( const QString &label ) { mLabel = label; }
    QString label() const { return mLabel; }

    void setWhatsThis( const QString &whatsThis ) { mWhatsThis = whatsThis; }
    QString whatsThis() const { return mWhatsThis; }

    void setDefaultValue( const QString &d ) { mDefaultValue = d; }
    QString defaultValue() const { return mDefaultValue; }

    void setCode( const QString &d ) { mCode = d; }
    QString code() const { return mCode; }

    void setMinValue( const QString &d ) { mMin = d; }
    QString minValue() const { return mMin; }

    void setMaxValue( const QString &d ) { mMax = d; }
    QString maxValue() const { return mMax; }

    void setParam( const QString &d ) { mParam = d; }
    QString param() const { return mParam; }

    void setParamName( const QString &d ) { mParamName = d; }
    QString paramName() const { return mParamName; }

    void setParamType( const QString &d ) { mParamType = d; }
    QString paramType() const { return mParamType; }

    void setChoices( const QList<Choice> &d, const QString &n, const QString &p ) { mChoices = Choices( d, n, p ); }
    Choices choices() const { return mChoices; }

    void setParamValues( const QStringList &d ) { mParamValues = d; }
    QStringList paramValues() const { return mParamValues; }

    void setParamDefaultValues( const QStringList &d ) { mParamDefaultValues = d; }
    QString paramDefaultValue(int i) const { return mParamDefaultValues[i]; }

    void setParamMax( int d ) { mParamMax = d; }
    int paramMax() const { return mParamMax; }

    void setSignalList( const QList<Signal> &value ) { mSignalList = value; }
    QList<Signal> signalList() const { return mSignalList; }

    bool hidden() const { return mHidden; }

    void dump() const
    {
      std::cerr << "<entry>" << std::endl;
      std::cerr << "  group: " << qPrintable(mGroup) << std::endl;
      std::cerr << "  type: " << qPrintable(mType) << std::endl;
      std::cerr << "  key: " << qPrintable(mKey) << std::endl;
      std::cerr << "  name: " << qPrintable(mName) << std::endl;
      std::cerr << "  context: " << qPrintable(mContext) << std::endl;
      std::cerr << "  label: " << qPrintable(mLabel) << std::endl;
// whatsthis
      std::cerr << "  code: " << qPrintable(mCode) << std::endl;
//      std::cerr << "  values: " << mValues.join(":") << std::endl;

      if (!param().isEmpty())
      {
        std::cerr << "  param name: "<< qPrintable(mParamName) << std::endl;
        std::cerr << "  param type: "<< qPrintable(mParamType) << std::endl;
        std::cerr << "  paramvalues: " << qPrintable(mParamValues.join(":")) << std::endl;
      }
      std::cerr << "  default: " << qPrintable(mDefaultValue) << std::endl;
      std::cerr << "  hidden: " << mHidden << std::endl;
      std::cerr << "  min: " << qPrintable(mMin) << std::endl;
      std::cerr << "  max: " << qPrintable(mMax) << std::endl;
      std::cerr << "</entry>" << std::endl;
    }

  private:
    QString mGroup;
    QString mType;
    QString mKey;
    QString mName;
    QString mContext;
    QString mLabel;
    QString mWhatsThis;
    QString mCode;
    QString mDefaultValue;
    QString mParam;
    QString mParamName;
    QString mParamType;
    Choices mChoices;
    QList<Signal> mSignalList;
    QStringList mParamValues;
    QStringList mParamDefaultValues;
    int mParamMax;
    bool mHidden;
    QString mMin;
    QString mMax;
};

class Param {
public:
  QString name;
  QString type;
};

// returns the name of an member variable
// use itemPath to know the full path
// like using d-> in case of dpointer
static QString varName(const QString &n)
{
  QString result;
  result = '@'+n;
  result[1] = result[1].toLower();
  return result;
}

static QString varPath(const QString &n)
{
  QString result;
  result = varName(n);
  return result;
}

static QString enumName(const QString &n)
{
  QString result = "Enum" + n;
  result[4] = result[4].toUpper();
  return result;
}

static QString enumName(const QString &n, const CfgEntry::Choices &c)
{
  QString result = c.name();
  if ( result.isEmpty() )
  {
    result = "Enum" + n;
    result[4] = result[4].toUpper();
  }
  return result;
}

static QString enumType(const CfgEntry *e)
{
  QString result = e->choices().name();
  if ( result.isEmpty() )
  {
    result = "Enum" + e->name() + ".type";
    result[4] = result[4].toUpper();
  }
  return result;
}

static QString enumTypeQualifier(const QString &n, const CfgEntry::Choices &c)
{
  QString result = c.name();
  if ( result.isEmpty() )
  {
    result = "Enum" + n + "::";
    result[4] = result[4].toUpper();
  }
  else if ( c.external() )
    result = c.externalQualifier();
  else
    result.clear();
  return result;
}

static QString enumValue(const QString &n)
{
  QString result = n;
  result[0] = result[0].toUpper();
  return result;
}

static QString setFunction(const QString &n, const QString &className = QString())
{
  QString result = "set"+n;
  result[3] = result[3].toUpper();

  if ( !className.isEmpty() )
    result = className + "." + result;
  return result;
}

static QString attrWriter(const QString &n, const QString &className = QString())
{
  QString result = n+"=";
  result[0] = result[0].toLower();

  if ( !className.isEmpty() )
    result = className + "." + result;
  return result;
}


static QString getFunction(const QString &n, const QString &className = QString())
{
  QString result = n;
  result[0] = result[0].toLower();

  if ( !className.isEmpty() )
    result = className + "." + result;
  return result;
}


static void addQuotes( QString &s )
{
  if ( !s.startsWith( '"' ) ) s.prepend( '"' );
  if ( !s.endsWith( '"' ) ) s.append( '"' );
}

static QString quoteString( const QString &s )
{
  QString r = s;
  r.replace( "\\", "\\\\" );
  r.replace( "\"", "\\\"" );
  r.replace( "\r", "" );
  r.replace( "\n", "\\n\"\n\"" );
  return "\"" + r + "\"";
}

static QString literalString( const QString &s )
{
  bool isAscii = true;
  for(int i = s.length(); i--;)
     if (s[i].unicode() > 127) isAscii = false;

  if (isAscii)
     return "" + quoteString(s) + "";
  else
     return "" + quoteString(s) + "";
}

static QString dumpNode(const QDomNode &node)
{
  QString msg;
  QTextStream s(&msg, QIODevice::WriteOnly );
  node.save(s, 0);

  msg = msg.simplified();
  if (msg.length() > 40)
    return msg.left(37)+"...";
  return msg;
}

static QString signalEnumName(const QString &signalName)
{
  QString result;
  result = "Signal" + signalName;
  result[6] = result[6].toUpper();

  return result;
}

static QString paramName(const QString &paramName)
{
  QString result;
  result = paramName;
  result[0] = result[0].toLower();

  return result;
}

static void preProcessDefault( QString &defaultValue, const QString &name,
                               const QString &type,
                               const CfgEntry::Choices &choices,
                               QString &code )
{
    if ( type == "String" && !defaultValue.isEmpty() ) {
      defaultValue = literalString(defaultValue);

    } else if ( type == "Path" && !defaultValue.isEmpty() ) {
      defaultValue = literalString( defaultValue );

    } else if ((type == "StringList" || type == "PathList") && !defaultValue.isEmpty()) {
      QTextStream rb( &code, QIODevice::WriteOnly | QIODevice::Append );
      if (!code.isEmpty())
         rb << endl;

      rb << "    default" << name << " = []" << endl;
      const QStringList defaults = defaultValue.split( "," );
      QStringList::ConstIterator it;
      for (it = defaults.begin(); it != defaults.end(); ++it) {
        rb << "    default" << name << " << \"" << *it << "\"" << endl;
      }
      defaultValue = "default" + name;

    } else if ( type == "Color" && !defaultValue.isEmpty() ) {
      QRegExp colorRe("\\d+,\\s*\\d+,\\s*\\d+(,\\s*\\d+)?");
      if (colorRe.exactMatch(defaultValue))
      {
        defaultValue = "Qt::Color.new(" + defaultValue + ")";
      }
      else
      {
        defaultValue = "Qt::Color.new(\"" + defaultValue + "\")";
      }

    } else if ( type == "Enum" ) {
      QList<CfgEntry::Choice>::ConstIterator it;
      for( it = choices.choices.begin(); it != choices.choices.end(); ++it ) {
        if ( (*it).name == defaultValue ) {
          if ( globalEnums && choices.name().isEmpty() )
            defaultValue.prepend( choices.prefix );
          else
            defaultValue.prepend( enumTypeQualifier(name, choices) + choices.prefix );
          break;
        }
      }

    } else if ( type == "IntList" ) {
      QTextStream rb( &code, QIODevice::WriteOnly | QIODevice::Append );
      if (!code.isEmpty())
         rb << endl;

      rb << "  default" << name << " = []" << endl;
      if (!defaultValue.isEmpty())
      {
        QStringList defaults = defaultValue.split( "," );
        QStringList::ConstIterator it;
        for (it = defaults.begin(); it != defaults.end(); ++it) {
          rb << "  default" << name << " << " << *it << "" << endl;
        }
      }
      defaultValue = "default" + name;
    }
}


CfgEntry *parseEntry( const QString &group, const QDomElement &element )
{
  bool defaultCode = false;
  QString type = element.attribute( "type" );
  QString name = element.attribute( "name" );
  QString key = element.attribute( "key" );
  QString hidden = element.attribute( "hidden" );
  QString context = element.attribute( "context" );
  QString label;
  QString whatsThis;
  QString defaultValue;
  QString code;
  QString param;
  QString paramName;
  QString paramType;
  CfgEntry::Choices choices;
  QList<Signal> signalList;
  QStringList paramValues;
  QStringList paramDefaultValues;
  QString minValue;
  QString maxValue;
  int paramMax = 0;

  for ( QDomElement e = element.firstChildElement(); !e.isNull(); e = e.nextSiblingElement() ) {
    QString tag = e.tagName();
    if ( tag == "label" ) {
      label = e.text();
      context = e.attribute( "context" );
    }
    else if ( tag == "whatsthis" ) {
      whatsThis = e.text();
      context = e.attribute( "context" );
    }
    else if ( tag == "min" ) minValue = e.text();
    else if ( tag == "max" ) maxValue = e.text();
    else if ( tag == "code" ) code = e.text();
    else if ( tag == "parameter" )
    {
      param = e.attribute( "name" );
      paramType = e.attribute( "type" );
      if ( param.isEmpty() ) {
        std::cerr << "Parameter must have a name: " << qPrintable(dumpNode(e)) << std::endl;
        return 0;
      }
      if ( paramType.isEmpty() ) {
        std::cerr << "Parameter must have a type: " << qPrintable(dumpNode(e)) << std::endl;
        return 0;
      }
      if ((paramType == "Int") || (paramType == "UInt"))
      {
         bool ok;
         paramMax = e.attribute("max").toInt(&ok);
         if (!ok)
         {
           std::cerr << "Integer parameter must have a maximum (e.g. max=\"0\"): "
                       << qPrintable(dumpNode(e)) << std::endl;
           return 0;
         }
      }
      else if (paramType == "Enum")
      {
         for ( QDomElement e2 = e.firstChildElement(); !e2.isNull(); e2 = e2.nextSiblingElement() ) {
           if (e2.tagName() == "values")
           {
             for ( QDomElement e3 = e2.firstChildElement(); !e3.isNull(); e3 = e3.nextSiblingElement() ) {
               if (e3.tagName() == "value")
               {
                  paramValues.append( e3.text() );
               }
             }
             break;
           }
         }
         if (paramValues.isEmpty())
         {
           std::cerr << "No values specified for parameter '" << qPrintable(param)
                       << "'." << std::endl;
           return 0;
         }
         paramMax = paramValues.count()-1;
      }
      else
      {
        std::cerr << "Parameter '" << qPrintable(param) << "' has type " << qPrintable(paramType)
                    << " but must be of type int, uint or Enum." << std::endl;
        return 0;
      }
    }
    else if ( tag == "default" )
    {
      if (e.attribute("param").isEmpty())
      {
        defaultValue = e.text();
        if (e.attribute( "code" ) == "true")
          defaultCode = true;
      }
    }
    else if ( tag == "choices" ) {
      QString name = e.attribute( "name" );
      QString prefix = e.attribute( "prefix" );
      QList<CfgEntry::Choice> chlist;
      for( QDomElement e2 = e.firstChildElement(); !e2.isNull(); e2 = e2.nextSiblingElement() ) {
        if ( e2.tagName() == "choice" ) {
          CfgEntry::Choice choice;
          choice.name = e2.attribute( "name" );
          if ( choice.name.isEmpty() ) {
            std::cerr << "Tag <choice> requires attribute 'name'." << std::endl;
          }
          for( QDomElement e3 = e2.firstChildElement(); !e3.isNull(); e3 = e3.nextSiblingElement() ) {
            if ( e3.tagName() == "label" ) {
              choice.label = e3.text();
              choice.context = e3.attribute( "context" );
            }
            if ( e3.tagName() == "whatsthis" ) {
              choice.whatsThis = e3.text();
              choice.context = e3.attribute( "context" );
            }
          }
          chlist.append( choice );
        }
      }
      choices = CfgEntry::Choices( chlist, name, prefix );
    }
   else if ( tag == "emit" ) {
    QDomNode signalNode;
    Signal signal;
    signal.name = e.attribute( "signal" );
    signalList.append( signal);
   }
  }


  bool nameIsEmpty = name.isEmpty();
  if ( nameIsEmpty && key.isEmpty() ) {
    std::cerr << "Entry must have a name or a key: " << qPrintable(dumpNode(element)) << std::endl;
    return 0;
  }

  if ( key.isEmpty() ) {
    key = name;
  }

  if ( nameIsEmpty ) {
    name = key;
    name.replace( " ", QString() );
  } else if ( name.contains( ' ' ) ) {
      std::cout<<"Entry '"<<qPrintable(name)<<"' contains spaces! <name> elements can't contain spaces!"<<std::endl;
    name.remove( ' ' );
  }

  if (name.contains("$("))
  {
    if (param.isEmpty())
    {
      std::cerr << "Name may not be parameterized: " << qPrintable(name) << std::endl;
      return 0;
    }
  }
  else
  {
    if (!param.isEmpty())
    {
      std::cerr << "Name must contain '$(" << qPrintable(param) << ")': " << qPrintable(name) << std::endl;
      return 0;
    }
  }

  if ( label.isEmpty() ) {
    label = key;
  }

  if ( type.isEmpty() ) type = "String"; // XXX : implicit type might be bad

  if (!param.isEmpty())
  {
    // Adjust name
    paramName = name;
    name.replace("$("+param+')', QString());
    // Lookup defaults for indexed entries
    for(int i = 0; i <= paramMax; i++)
    {
      paramDefaultValues.append(QString());
    }

    for ( QDomElement e = element.firstChildElement(); !e.isNull(); e = e.nextSiblingElement() ) {
      QString tag = e.tagName();
      if ( tag == "default" )
      {
        QString index = e.attribute("param");
        if (index.isEmpty())
           continue;

        bool ok;
        int i = index.toInt(&ok);
        if (!ok)
        {
          i = paramValues.indexOf(index);
          if (i == -1)
          {
            std::cerr << "Index '" << qPrintable(index) << "' for default value is unknown." << std::endl;
            return 0;
          }
        }

        if ((i < 0) || (i > paramMax))
        {
          std::cerr << "Index '" << i << "' for default value is out of range [0, "<< paramMax<<"]." << std::endl;
          return 0;
        }

        QString tmpDefaultValue = e.text();

        if (e.attribute( "code" ) != "true")
           preProcessDefault(tmpDefaultValue, name, type, choices, code);

        paramDefaultValues[i] = tmpDefaultValue;
      }
    }
  }

  if (!validNameRegexp->exactMatch(name))
  {
    if (nameIsEmpty)
      std::cerr << "The key '" << qPrintable(key) << "' can not be used as name for the entry because "
                   "it is not a valid name. You need to specify a valid name for this entry." << std::endl;
    else
      std::cerr << "The name '" << qPrintable(name) << "' is not a valid name for an entry." << std::endl;
    return 0;
  }

  if (allNames.contains(name))
  {
    if (nameIsEmpty)
      std::cerr << "The key '" << qPrintable(key) << "' can not be used as name for the entry because "
                   "it does not result in a unique name. You need to specify a unique name for this entry." << std::endl;
    else
      std::cerr << "The name '" << qPrintable(name) << "' is not unique." << std::endl;
    return 0;
  }
  allNames.append(name);

  if (!defaultCode)
  {
    preProcessDefault(defaultValue, name, type, choices, code);
  }

  CfgEntry *result = new CfgEntry( group, type, key, name, context, label, whatsThis,
                                   code, defaultValue, choices, signalList,
                                   hidden == "true" );
  if (!param.isEmpty())
  {
    result->setParam(param);
    result->setParamName(paramName);
    result->setParamType(paramType);
    result->setParamValues(paramValues);
    result->setParamDefaultValues(paramDefaultValues);
    result->setParamMax(paramMax);
  }
  result->setMinValue(minValue);
  result->setMaxValue(maxValue);

  return result;
}

/**
  Return parameter declaration for given type.
*/
QString param( const QString &type )
{
    if ( type == "String" )           return "const QString &";
    else if ( type == "StringList" )  return "const QStringList &";
    else if ( type == "Font" )        return "const QFont &";
    else if ( type == "Rect" )        return "const QRect &";
    else if ( type == "Size" )        return "const QSize &";
    else if ( type == "Color" )       return "const QColor &";
    else if ( type == "Point" )       return "const QPoint &";
    else if ( type == "Int" )         return "int";
    else if ( type == "UInt" )        return "uint";
    else if ( type == "Bool" )        return "bool";
    else if ( type == "Double" )      return "double";
    else if ( type == "DateTime" )    return "const QDateTime &";
    else if ( type == "LongLong" )    return "qint64";
    else if ( type == "ULongLong" )   return "quint64";
    else if ( type == "IntList" )     return "const QList<int> &";
    else if ( type == "Enum" )        return "int";
    else if ( type == "Path" )        return "const QString &";
    else if ( type == "PathList" )    return "const QStringList &";
    else if ( type == "Password" )    return "const QString &";
    else if ( type == "Url" )         return "const KUrl &";
    else if ( type == "UrlList" )     return "const KUrl::List &";
    else {
        std::cerr <<"kconfig_compiler does not support type \""<< type <<"\""<<std::endl;
        return "QString"; //For now, but an assert would be better
    }
}

/**
  Actual Ruby initializer value to give a type
*/
QString rbType( const QString &type )
{
    if ( type == "String" )           return "\"\"";
    else if ( type == "StringList" )  return "[]";
    else if ( type == "Font" )        return "Qt::Font.new";
    else if ( type == "Rect" )        return "Qt::Rect.new";
    else if ( type == "Size" )        return "Qt::Size.new";
    else if ( type == "Color" )       return "Qt::Color.new";
    else if ( type == "Point" )       return "Qt::Point.new";
    else if ( type == "Int" )         return "0";
    else if ( type == "UInt" )        return "0";
    else if ( type == "Bool" )        return "false";
    else if ( type == "Double" )      return "0.0";
    else if ( type == "DateTime" )    return "Qt::DateTime.new";
    else if ( type == "LongLong" )    return "0";
    else if ( type == "ULongLong" )   return "0";
    else if ( type == "IntList" )     return "[]";
    else if ( type == "Enum" )        return "0";
    else if ( type == "Path" )        return "\"\"";
    else if ( type == "PathList" )    return "[]";
    else if ( type == "Password" )    return "\"\"";
    else if ( type == "Url" )         return "KDE::Url.new";
    else if ( type == "UrlList" )     return "[]";
    else {
        std::cerr<<"kconfig_compiler does not support type \""<< type <<"\""<<std::endl;
        return "\"\""; //For now, but an assert would be better
    }
}

QString defaultValue( const QString &type )
{
    if ( type == "String" )           return "\"\""; // Use empty string, not null string!
    else if ( type == "StringList" )  return "[]";
    else if ( type == "Font" )        return "Qt::Font.new";
    else if ( type == "Rect" )        return "Qt::Rect.new";
    else if ( type == "Size" )        return "Qt::Size.new";
    else if ( type == "Color" )       return "Qt::Color.new(128, 128, 128)";
    else if ( type == "Point" )       return "Qt::Point.new";
    else if ( type == "Int" )         return "0";
    else if ( type == "UInt" )        return "0";
    else if ( type == "Bool" )        return "false";
    else if ( type == "Double" )      return "0.0";
    else if ( type == "DateTime" )    return "Qt::DateTime.new";
    else if ( type == "LongLong" )    return "0";
    else if ( type == "ULongLong" )   return "0";
    else if ( type == "IntList" )     return "[]";
    else if ( type == "Enum" )        return "0";
    else if ( type == "Path" )        return "\"\""; // Use empty string, not null string!
    else if ( type == "PathList" )    return "[]";
    else if ( type == "Password" )    return "\"\""; // Use empty string, not null string!
    else if ( type == "Url" )         return "KDE::Url.new";
    else if ( type == "UrlList" )     return "[]";
    else {
        std::cerr<<"Error, kconfig_compiler doesn't support the \""<< type <<"\" type!"<<std::endl;
        return "QString"; //For now, but an assert would be better
    }
}

QString itemType( const QString &type )
{
  QString t;

  t = type;
  t.replace( 0, 1, t.left( 1 ).toUpper() );

  return t;
}

// returns the name of an item variable
// use itemPath to know the full path
// like using d-> in case of dpointer
static QString itemVar(const CfgEntry *e)
{
  QString result;
  if (itemAccessors) {
    result = '@' + e->name() + "Item";
    result[1] = result[1].toLower();
  } else {
    result = "item" + e->name();
    result[4] = result[4].toUpper();
  }
  return result;
}

static QString itemPath(const CfgEntry *e)
{
  QString result;
  result = itemVar(e);
  return result;
}

QString newItem( const QString &type, const QString &name, const QString &key,
                 const QString &defaultValue, const QString &param = QString())
{
    QString t = "Item" + itemType( type ) +
                ".new(currentGroup(), " + key + ", " + varPath( name ) + param;
    if (type == "Enum") {
        t += ".toInt";
        t += ", values" + name;
    } else if ( type == "Path" ) {
        t += ".toString";  
    } else if ( type == "Int64" ) {
        t += ".toLongLong";  
    } else {
        t += ".value";
    }
    if (!defaultValue.isEmpty()) {
        t += ", ";
        if (type == "Enum") {
            t += enumValue(defaultValue);
        } else {
            t += defaultValue;
        }
    }
    t += ")";

    return t;
}

QString paramString(const QString &s, const CfgEntry *e, int i)
{
    QString result = s;
    QString needle = "$("+e->param()+')';
    if (result.contains(needle)) {
        QString tmp;
        if (e->paramType() == "Enum") {
            tmp = e->paramValues()[i];
        } else {
            tmp = QString::number(i);
        }

        result.replace(needle, tmp);
    }
    return result;
}

QString paramString(const QString &group, const QList<Param> &parameters)
{
  QString paramString = group;
  QString arguments;
  for ( QList<Param>::ConstIterator it = parameters.begin();
        it != parameters.end(); 
        ++it )
  {
     if (paramString.contains("$(" + (*it).name+')')) {
       paramString.replace("$(" + (*it).name +')', "#{@param" + paramName((*it).name) + ".value}");
       arguments += "@param" + paramName((*it).name) + ", ";
     }
  }
  if (arguments.isEmpty())
    return "\"" + group + "\"";

  return "\"" + paramString + "\"";
}

/* int i is the value of the parameter */
QString userTextsFunctions( CfgEntry *e, QString itemVarStr=QString(), QString i=QString() )
{
  QString txt;
  if (itemVarStr.isNull()) itemVarStr=itemPath(e);
  if ( !e->label().isEmpty() ) {
    txt += "    " + itemVarStr + ".label = ";
    if ( !e->context().isEmpty() )
      txt += "i18nc(" + quoteString(e->context()) + ", ";
    else
      txt += "i18n(";
    if ( !e->param().isEmpty() )
      txt += quoteString(e->label().replace("$("+e->param()+')', i));
    else
      txt+= quoteString(e->label());
    txt+= ")\n";
  }
  if ( !e->whatsThis().isEmpty() ) {
    txt += "    " + itemVarStr + ".whatsThis = ";
    if ( !e->context().isEmpty() )
      txt += "i18nc(" + quoteString(e->context()) + ", ";
    else
      txt += "i18n(";
    if ( !e->param().isEmpty() )
      txt += quoteString(e->whatsThis().replace("$("+e->param()+')', i));
    else
      txt+= quoteString(e->whatsThis());
    txt+=")\n";
  }
  return txt;
}

// returns the member accesor implementation
// which should go in the h file if inline
// or the cpp file if not inline
QString memberAccessorBody( CfgEntry *e )
{
    QString result;
    QTextStream out(&result, QIODevice::WriteOnly);
    QString n = e->name();
    QString t = e->type();

    out << varName(n); 
    if (!e->param().isEmpty()) {
        out << "[i]";
    }
    out << " = findItem(\"";
    if (!e->param().isEmpty()) {
      out << e->paramName().replace("$("+e->param()+")", "%s") << "\" % ";
      if (e->paramType() == "Enum") {
        out << " ";
        if (globalEnums) 
          out << enumName(e->param()) << "ToString(i)";
        else 
          out << enumName(e->param()) << ".enumToString(i)";
      } else {
        out << "i";
      }
    } else {
      out << n << "\"";
    }
    out << ").property" << endl;

    out << "return " << varPath(n);
    if (!e->param().isEmpty()) {
        out << "[i]";
    }
    if (t == "Enum") {
      out << ".toInt" << endl;
    } else if (t == "Int64") {
      out << ".toLongLong" << endl;
    } else if (t == "Path") {
      out << ".toString" << endl;
    } else {
      out << ".value" << endl;
    }

    return result;
}

// returns the member mutator implementation
// which should go in the h file if inline
// or the cpp file if not inline
QString memberMutatorBody(CfgEntry *e)
{
    QString result;
    QTextStream out(&result, QIODevice::WriteOnly);
    QString n = e->name();
    QString t = e->type();

    if (!e->minValue().isEmpty()) {
        out << "if v < " << e->minValue() << "" << endl;
        out << "  puts \"" << setFunction(n);
        out << ": value #{v} is less than the minimum value of ";
        out << e->minValue()<< "\"" << endl;
        out << "  v = " << e->minValue() << "" << endl;
        out << "end" << endl;
    }

    if (!e->maxValue().isEmpty()) {
        out << endl << "if v > " << e->maxValue() << "" << endl;
        out << "  puts \"" << setFunction(n);
        out << ": value #{v} is greater than the maximum value of ";
        out << e->maxValue()<< "\"" << endl;
        out << "  v = " << e->maxValue() << "" << endl;
        out << "end" << endl << endl;
    }

    out << "item = findItem(\"";
    if (!e->param().isEmpty()) {
        out << e->paramName().replace("$("+e->param()+")", "%s") << "\" % ";
        if (e->paramType() == "Enum") {
            out << "";

            if (globalEnums) {
                out << enumName(e->param()) << "ToString(i)";
            } else {
                out << enumName(e->param()) << ".enumToString(i)";
            }
        } else {
            out << "i";
        }
        out << ")";
    } else {
        out << n << "\")";
    }
    out << endl;
    out << "if !item.immutable?" << endl;
    out << "  item.property = " << varName(n);
    if (!e->param().isEmpty()) {
        out << "[i]";
    }

    out << " = Qt::Variant.fromValue(v)" << endl;

    if (!e->signalList().empty()) {
        foreach(const Signal &signal, e->signalList()) {
            out << "  " << varPath("settingsChanged") << " |= " << signalEnumName(signal.name) << "" << endl;
        }
    }
    out << "end" << endl;

    return result;
}

// returns the item accesor implementation
// which should go in the h file if inline
// or the cpp file if not inline
QString itemAccessorBody( CfgEntry *e )
{
    QString result;
    QTextStream out(&result, QIODevice::WriteOnly);

    out << "return " << itemPath(e);
    if (!e->param().isEmpty()) out << "[i]";
    out << "" << endl;

    return result;
}

//indents text adding X spaces per line
QString indent(QString text, int spaces)
{
    QString result;
    QTextStream out(&result, QIODevice::WriteOnly);
    QTextStream in(&text, QIODevice::ReadOnly);
    QString currLine;
    while ( !in.atEnd() )
    {
      currLine = in.readLine();
      if (!currLine.isEmpty())
        for (int i=0; i < spaces; i++)
          out << " ";
      out << currLine << endl;
    }
    return result;
}


int main( int argc, char **argv )
{
  QCoreApplication app(argc, argv);

  validNameRegexp = new QRegExp("[a-zA-Z_][a-zA-Z0-9_]*");

  QString directoryName, inputFilename, codegenFilename;
  parseArgs(app.arguments(), directoryName, inputFilename, codegenFilename);

  QString baseDir = directoryName;
#ifdef Q_OS_WIN
  if (!baseDir.endsWith('/') && !baseDir.endsWith('\\'))
#else
  if (!baseDir.endsWith('/'))
#endif
    baseDir.append("/");

  if (!codegenFilename.endsWith(".kcfgc"))
  {
    std::cerr << "Codegen options file must have extension .kcfgc" << std::endl;
    return 1;
  }
  QString baseName = QFileInfo(codegenFilename).fileName();
  baseName = baseName.left(baseName.length() - 6);

  QSettings codegenConfig(codegenFilename, QSettings::IniFormat);

  QString nameSpace = codegenConfig.value("NameSpace").toString();
  QString className = codegenConfig.value("ClassName").toString();
  QString inherits = codegenConfig.value("Inherits").toString();
  QString visibility = codegenConfig.value("Visibility").toString();
  if (!visibility.isEmpty()) visibility+=' ';
  bool singleton = codegenConfig.value("Singleton", false).toBool();
  QString memberVariables = codegenConfig.value("MemberVariables").toString();
  QStringList headerIncludes = codegenConfig.value("IncludeFiles", QStringList()).toStringList();
  QStringList sourceIncludes = codegenConfig.value("SourceIncludeFiles", QStringList()).toStringList();
  QStringList mutators = codegenConfig.value("Mutators", QStringList()).toStringList();
  bool allMutators = false;
  if ((mutators.count() == 1) && (mutators.at(0).toLower() == "true"))
     allMutators = true;
  itemAccessors = codegenConfig.value("ItemAccessors", false).toBool();
  bool setUserTexts = codegenConfig.value("SetUserTexts", false).toBool();

  globalEnums = codegenConfig.value("GlobalEnums", false).toBool();
  useEnumTypes = codegenConfig.value("UseEnumTypes", false).toBool();

  QFile input( inputFilename );

  QDomDocument doc;
  QString errorMsg;
  int errorRow;
  int errorCol;
  if ( !doc.setContent( &input, &errorMsg, &errorRow, &errorCol ) ) {
    std::cerr << "Unable to load document." << std::endl;
    std::cerr << "Parse error in " << inputFilename << ", line " << errorRow << ", col " << errorCol << ": " << errorMsg << std::endl;
    return 1;
  }

  QDomElement cfgElement = doc.documentElement();

  if ( cfgElement.isNull() ) {
    std::cerr << "No document in kcfg file" << std::endl;
    return 1;
  }

  QString cfgFileName;
  bool cfgFileNameArg = false;
  QList<Param> parameters;
  QList<Signal> signalList;
  QStringList includes;
  bool hasSignals = false;

  QList<CfgEntry*> entries;

  for ( QDomElement e = cfgElement.firstChildElement(); !e.isNull(); e = e.nextSiblingElement() ) {
    QString tag = e.tagName();

    if ( tag == "include" ) {
      QString includeFile = e.text();
      if (!includeFile.isEmpty())
        includes.append(includeFile);

    } else if ( tag == "kcfgfile" ) {
      cfgFileName = e.attribute( "name" );
      cfgFileNameArg = e.attribute( "arg" ).toLower() == "true";
      for( QDomElement e2 = e.firstChildElement(); !e2.isNull(); e2 = e2.nextSiblingElement() ) {
        if ( e2.tagName() == "parameter" ) {
          Param p;
          p.name = e2.attribute( "name" );
          p.type = e2.attribute( "type" );
          if (p.type.isEmpty())
             p.type = "String";
          parameters.append( p );
        }
      }

    } else if ( tag == "group" ) {
      QString group = e.attribute( "name" );
      if ( group.isEmpty() ) {
        std::cerr << "Group without name" << std::endl;
        return 1;
      }
      for( QDomElement e2 = e.firstChildElement(); !e2.isNull(); e2 = e2.nextSiblingElement() ) {
        if ( e2.tagName() != "entry" ) continue;
        CfgEntry *entry = parseEntry( group, e2 );
        if ( entry ) entries.append( entry );
        else {
          std::cerr << "Can't parse entry." << std::endl;
          return 1;
        }
      }
    }
    else if ( tag == "signal" ) {
      QString signalName = e.attribute( "name" );
      if ( signalName.isEmpty() ) {
        std::cerr << "Signal without name." << std::endl;
        return 1;
      }
      Signal theSignal;
      theSignal.name = signalName;

      for( QDomElement e2 = e.firstChildElement(); !e2.isNull(); e2 = e2.nextSiblingElement() ) {
        if ( e2.tagName() == "argument") {
          SignalArguments argument;
          argument.type = e2.attribute("type");
          if ( argument.type.isEmpty() ) {
            std::cerr << "Signal argument without type." << std::endl;
            return 1;
          }
          argument.variableName = e2.text();
          theSignal.arguments.append(argument);
        }
        else if( e2.tagName() == "label") {
          theSignal.label = e2.text();
        }
      }
      signalList.append(theSignal);
    }
  }

  if ( inherits.isEmpty() ) inherits = "KDE::ConfigSkeleton";

  if ( className.isEmpty() ) {
    std::cerr << "Class name missing" << std::endl;
    return 1;
  }

  if ( singleton && !parameters.isEmpty() ) {
    std::cerr << "Singleton class can not have parameters" << std::endl;
    return 1;
  }

  if ( !cfgFileName.isEmpty() && cfgFileNameArg)
  {
    std::cerr << "Having both a fixed filename and a filename as argument is not possible." << std::endl;
    return 1;
  }

  if ( entries.isEmpty() ) {
    std::cerr << "No entries." << std::endl;
  }

#if 0
  CfgEntry *cfg;
  for( cfg = entries.first(); cfg; cfg = entries.next() ) {
    cfg->dump();
  }
#endif

    hasSignals = !signalList.empty();
    QString implementationFileName = baseName + ".rb";

    QFile implementation(baseDir + implementationFileName);
    if (!implementation.open(QIODevice::WriteOnly)) {
        std::cerr << "Can't open '" << qPrintable(implementationFileName) << "for writing." << std::endl;
        return 1;
    }

    QTextStream rb(&implementation);

    rb << "# This file is generated by rbkconfig_compiler from " << QFileInfo(inputFilename).fileName() << "." << endl;
    rb << "# All changes you do to this file will be lost." << endl << endl;

    if (singleton) {
        rb << "require 'singleton'" << endl << endl;
    }

    if (!nameSpace.isEmpty()) {
        rb << "module " << nameSpace << endl << endl;
    }

    rb << "class " << className << " < " << inherits << endl;

    if (singleton) {
        rb << "  include Singleton" << endl;
    }

    // enums
    QList<CfgEntry*>::ConstIterator itEntry;

    for (itEntry = entries.begin(); itEntry != entries.end(); ++itEntry) {
        const CfgEntry::Choices &choices = (*itEntry)->choices();
        QList<CfgEntry::Choice> chlist = choices.choices;

        if (!chlist.isEmpty()) {
            QStringList values;
            for (    QList<CfgEntry::Choice>::ConstIterator itChoice = chlist.begin();
                    itChoice != chlist.end(); 
                    ++itChoice ) 
            {
                values.append(choices.prefix + (*itChoice).name);
            }

            if (choices.name().isEmpty()) {
                if (globalEnums) {
                    int count = 0;
                    foreach (const QString& value, values) {
                        rb << "  " << enumValue(value) << " = " << count << endl;
                        count++;
                    }
                    rb << endl;
                } else {
                    // Create an automatically named enum
                    rb << endl;
                    rb << "  class " << enumName((*itEntry)->name(), (*itEntry)->choices()) << endl;
                    int count = 0;
                    foreach (const QString& value, values) {
                        rb << "    " << enumValue(value) << " = " << count << endl;
                        count++;
                    }
                    rb << "    COUNT = " << count << endl;
                    rb << "  end" << endl;
                }
            } else if (!choices.external()) {
                // Create a named enum
                int count = 0;
                foreach (const QString& value, values) {
                    rb << "  " << enumValue(value) << " = " << count << endl;
                    count++;
                }
                rb << endl;
            }
        }

        QStringList values = (*itEntry)->paramValues();
        if (!values.isEmpty()) {
            if (globalEnums) {
                int count = 0;
                foreach (const QString& value, values) {
                    rb << "  " << enumValue(value) << " = " << count << endl;
                    count++;
                }
                rb << endl;

                rb << "  def " << enumName((*itEntry)->param()) << "ToString(i)" << endl;
                rb << "    [";
                count = 0;
                foreach (const QString& value, values) {
                    if (count > 0) rb << ", ";
                    rb << "\"" << enumValue(value) << "\"";
                    count++;
                }
      
                rb << "].at(i)" << endl;
                rb << "  end" << endl;
            } else {
                rb << endl;
                rb << "  class " << enumName((*itEntry)->param()) << endl;
                int count = 0;
                foreach (const QString& value, values) {
                    rb << "    " << enumValue(value) << " = " << count << endl;
                    count++;
                }
                rb << "    COUNT = " << count << endl;

                rb << endl;
                rb << "    def enumToString(i)" << endl;
                rb << "      [";
                count = 0;
                foreach (const QString& value, values) {
                    if (count > 0) rb << ", ";
                    rb << "\"" << enumValue(value) << "\"";
                    count++;
                }
      
                rb << "].at(i)" << endl;
                rb << "    end" << endl;

                rb << "  end" << endl;

            }
        }
    }

    if (hasSignals) {
        rb << endl;
        unsigned val = 1;
        QList<Signal>::ConstIterator it, itEnd = signalList.constEnd();
        for (it = signalList.constBegin(); it != itEnd; val <<= 1) {
            if (val == 0) {
                std::cerr << "Too many signals to create unique bit masks" << std::endl;
                exit(1);
            }
            Signal signal = *it;
            rb << "  " << signalEnumName(signal.name) << " = 0x" << hex << val;
            if (++it != itEnd) {
                rb << ",";
            }
        rb << endl;
        }
    }

    rb << endl;

    for (itEntry = entries.begin(); itEntry != entries.end(); ++itEntry) {
        QString n = (*itEntry)->name();
        QString t = (*itEntry)->type();

        // Manipulator
        if (allMutators || mutators.contains(n)) {
            rb << "  #" << endl;
            rb << "  # Set " << (*itEntry)->label() << endl;
            rb << "  #" << endl;
            rb << "  def " << setFunction(n) << "(";
            if (!(*itEntry)->param().isEmpty()) {
                rb << "i, ";
            }
            rb << "v)";
            rb  << endl;
            rb << indent(memberMutatorBody(*itEntry), 4);
            rb << "  end" << endl;

            if ((*itEntry)->param().isEmpty()) {
                rb << endl;
                rb << "  def " << attrWriter(n) << "(v)" << endl;
                rb << "    " << setFunction(n) << "(v)" << endl;
                rb << "  end" << endl;
            }

            rb << endl;
        }

        // Accessor
        rb << "  #" << endl;
        rb << "  # Get " << (*itEntry)->label() << endl;
        rb << "  #" << endl;
        rb << "  def " << getFunction(n) << "";
        if (!(*itEntry)->param().isEmpty()) {
            rb <<"(i)";
        }
        rb << endl;
        rb << indent(memberAccessorBody((*itEntry)), 4);
        rb << "  end" << endl;
        rb << endl;

        // Item accessor
        if (itemAccessors) {
            rb << "  #" << endl;
            rb << "  # Get Item object corresponding to " << n << "()" << endl;
            rb << "  #" << endl;
            rb << "  def " << getFunction( n ) << "Item";
            if (!(*itEntry)->param().isEmpty()) {
                rb << "(i)";
            }
            rb << "";
            rb  << endl;
            rb << indent( itemAccessorBody((*itEntry)), 4);
            rb << "  end" << endl;
            rb << endl;
        }
    }

    // Signal definition.
    if (hasSignals) {
        rb << endl;
        foreach(const Signal &signal, signalList) {
            rb << endl;
            if (!signal.label.isEmpty()) {
                rb << "  #" << endl;
                rb << "  # " << signal.label << endl;
                rb << "  #" << endl;
            }
            rb << "  signals '" << signal.name << "(";
            QList<SignalArguments>::ConstIterator it, itEnd = signal.arguments.constEnd();
            for (it = signal.arguments.constBegin(); it != itEnd;) {
                SignalArguments argument = *it;
                QString type = param(argument.type);
                if (useEnumTypes && argument.type == "Enum") {
                    for (int i = 0, end = entries.count(); i < end; ++i) {
                        if ( entries[i]->name() == argument.variableName ) {
                            type = enumType(entries[i]);
                            break;
                        }
                    }
                }
                rb << type << " " << argument.variableName;
                if (++it != itEnd) {
                    rb << ", ";
                }
            }
            rb << ")'" << endl;
        }
        rb << endl;
    }

    // Constructor
    rb << "  def initialize" << "(";
    if (cfgFileNameArg) {
        rb << "config = KDE::Global.config";
        rb << (parameters.isEmpty() ? "" : ", ");
    }

    for (QList<Param>::ConstIterator it = parameters.begin(); it != parameters.end(); ++it) {
        if (it != parameters.begin()) {
            rb << ", ";
        }
        rb << paramName((*it).name);
    }

    rb << ")" << endl;

    rb << "    super(";
    if (!cfgFileName.isEmpty()) {
        rb << "\"" << cfgFileName << "\"";
    }

    if (cfgFileNameArg) {
        rb << "config";
    }

    rb << ")" << endl;

    // Store parameters
    for (QList<Param>::ConstIterator it = parameters.begin(); it != parameters.end(); ++it) {
        rb << "    @param" << paramName((*it).name);
        rb << " = Qt::Variant.fromValue(" << paramName((*it).name) << ")" << endl;
    }

    QString group;

    for (itEntry = entries.begin(); itEntry != entries.end(); ++itEntry) {
        if ((*itEntry)->group() != group) {
            group = (*itEntry)->group();
            rb << endl;
            rb << "    # " << group << endl;
        }

        if ((*itEntry)->param().isEmpty()) {
            rb << "    " << varName((*itEntry)->name()) << " = Qt::Variant.fromValue(" << rbType((*itEntry)->type()) << ")" << endl;
        } else {
            rb << "    " << varName((*itEntry)->name()) << " = [";
            for (int i = 0; i < (*itEntry)->paramMax()+1; i++) {
                if (i > 0) rb << ", ";
                rb << "Qt::Variant.fromValue(" << rbType((*itEntry)->type()) << ")";
            }
            rb << "]" << endl;
        }
    }

    if (hasSignals) {
        rb << "    " << varName("settingsChanged") << " = 0" << endl;
    }

    group.clear();

    for (itEntry = entries.begin(); itEntry != entries.end(); ++itEntry) {
        if ((*itEntry)->group() != group) {
            if (!group.isEmpty()) {
                rb << endl;
            }
            group = (*itEntry)->group();
            rb << "    setCurrentGroup(" << paramString(group, parameters) << ")" << endl << endl;
        }

        QString key = paramString((*itEntry)->key(), parameters);
        if (!(*itEntry)->code().isEmpty()) {
            rb << (*itEntry)->code() << endl;
        }

        if ((*itEntry)->type() == "Enum") {
            rb << "    values" << (*itEntry)->name() << " = []" << endl;
            QList<CfgEntry::Choice> choices = (*itEntry)->choices().choices;
            QList<CfgEntry::Choice>::ConstIterator it;
            for (it = choices.begin(); it != choices.end(); ++it) {
                rb << "    choice = ItemEnum::Choice.new" << endl;
                rb << "    choice.name = \"" << (*it).name << "\"" << endl;
                if (setUserTexts) {
                    if (!(*it).label.isEmpty()) {
                        rb << "    choice.label = ";
                        if ( !(*it).context.isEmpty() ) {
                            rb << "i18nc(" + quoteString((*it).context) + ", ";
                        } else {
                            rb << "i18n(";
                        }
                        rb << quoteString((*it).label) << ")" << endl;
                    }

                    if (!(*it).whatsThis.isEmpty()) {
                        rb << "    choice.whatsThis = ";
                        if (!(*it).context.isEmpty()) {
                            rb << "i18nc(" + quoteString((*it).context) + ", ";
                        } else {
                            rb << "i18n(";
                        }
                        rb << quoteString((*it).whatsThis) << ")" << endl;
                    }
                }
                rb << "    values" << (*itEntry)->name() << " << choice" << endl;
            }
        }

        if ((*itEntry)->param().isEmpty()) {
            // Normal case
            rb << "    " << itemPath( *itEntry ) << " = "
                << newItem( (*itEntry)->type(), (*itEntry)->name(), key, (*itEntry)->defaultValue() ) << endl;

            rb << "    " << itemVar(*itEntry) << ".property = " << varName((*itEntry)->name()) << endl;

            if (!(*itEntry)->minValue().isEmpty()) {
                rb << "    " << itemPath( *itEntry ) << ".minValue = " << (*itEntry)->minValue() << endl;
            }

            if (!(*itEntry)->maxValue().isEmpty()) {
                rb << "    " << itemPath( *itEntry ) << ".maxValue = " << (*itEntry)->maxValue() << endl;
            }

            if (setUserTexts) {
                rb << userTextsFunctions( (*itEntry) );
            }

            rb << "    addItem(" << itemPath( *itEntry );
            QString quotedName = (*itEntry)->name();
            addQuotes(quotedName);
            if ( quotedName != key ) {
                rb << ", \"" << (*itEntry)->name() << "\"";
            }
            rb << ")" << endl;
        } else {
            // Indexed
            rb << "    " << itemPath(*itEntry) << " = Array.new(" << (*itEntry)->paramMax()+1 << ")" << endl;
            for (int i = 0; i <= (*itEntry)->paramMax(); i++) {
                QString defaultStr;
                QString itemVarStr(itemPath( *itEntry )+QString("[%1]").arg(i));

                if (!(*itEntry)->paramDefaultValue(i).isEmpty()) {
                    defaultStr = (*itEntry)->paramDefaultValue(i);
                } else if (!(*itEntry)->defaultValue().isEmpty()) {
                    defaultStr = paramString( (*itEntry)->defaultValue(), (*itEntry), i );
                } else {
                    defaultStr = defaultValue( (*itEntry)->type() );
                }

                rb << "    " << itemVarStr << " = "
                    << newItem( (*itEntry)->type(), (*itEntry)->name(), paramString(key, *itEntry, i), defaultStr, QString("[%1]").arg(i) )
                    << endl;

                if (setUserTexts) {
                    rb << userTextsFunctions( *itEntry, itemVarStr, (*itEntry)->paramName() );
                }

                // Make mutators for enum parameters work by adding them with $(..) replaced by the
                // param name. The check for isImmutable in the set* functions doesn't have the param
                // name available, just the corresponding enum value (int), so we need to store the
                // param names in a separate static list!.
                rb << "    addItem(" << itemVarStr << ", \"";
                if ((*itEntry)->paramType()=="Enum") {
                    rb << (*itEntry)->paramName().replace( "$("+(*itEntry)->param()+')', "%1").arg(enumValue((*itEntry)->paramValues()[i]));
                } else {
                    rb << (*itEntry)->paramName().replace( "$("+(*itEntry)->param()+')', "%1").arg(i);
                }
                rb << "\")" << endl;
            }
        }
    }

    if (singleton) {
        rb << "    readConfig" << endl;
    }

    rb << "  end" << endl << endl;

    if (hasSignals) {
        rb << "  def usrWriteConfig" << endl;
        rb << "    " << "super" << endl;
        foreach(const Signal &signal, signalList) {
            rb << "    if " << varPath("settingsChanged") << " & " << signalEnumName(signal.name) << " != 0" << endl;
            rb << "      emit " << signal.name << "(";
            QList<SignalArguments>::ConstIterator it, itEnd = signal.arguments.constEnd();
            for (it = signal.arguments.constBegin(); it != itEnd;) {
                SignalArguments argument = *it;
                rb << varPath(argument.variableName) << ".value";
                if (++it != itEnd) {
                    rb << ", ";
                }
            }
            rb << ")" << endl;
            rb << "    end" << endl;
        }
        rb << "    " << varPath("settingsChanged") << " = 0" << endl;
        rb << "  end" << endl;
    }

    // clear entries list
    qDeleteAll( entries );

    rb << "end" << endl;

    if (!nameSpace.isEmpty())
        rb << "end" << endl;

    implementation.close();
}
