#include "command-line.h"
#include "log.h"
#include "config.h"
#include "global-value.h"
#include "string.h"
#include <stdlib.h>

NS_LOG_COMPONENT_DEFINE ("CommandLine");

namespace ns3 {

CommandLine::~CommandLine ()
{
  for (Items::const_iterator i = m_items.begin (); i != m_items.end (); ++i)
    {
      delete *i;
    }
  m_items.clear ();
}

CommandLine::Item::~Item ()
{}

void 
CommandLine::Parse (int &iargc, char *argv[]) const
{
  int argc = iargc;
  for (argc--, argv++; argc > 0; argc--, argv++)
    {
      // remove "--" or "-" heading.
      std::string param = *argv;
      std::string::size_type cur = param.find ("--");
      if (cur == 0)
	{
	  param = param.substr (2, param.size () - 2);
	}
      else
	{
	  cur = param.find ("-");
	  if (cur == 0)
	    {
	      param = param.substr (1, param.size () - 1);
	    }
	  else
	    {
	      // invalid argument. ignore.
	      continue;
	    }
	}
      cur = param.find ("=");
      std::string name, value;
      if (cur == std::string::npos)
        {
	  name = param;
	  value = "";
        }
      else
        {
          name = param.substr (0, cur);
          value = param.substr (cur + 1, param.size () - (cur+1));
        }
      HandleArgument (name, value);
    }
}

void
CommandLine::PrintHelp (void) const
{
  std::cout << "--PrintHelp: Print this help message." << std::endl;
  std::cout << "--PrintGroups: Print the list of groups." << std::endl;
  std::cout << "--PrintTypeIds: Print all TypeIds." << std::endl;  
  std::cout << "--PrintGroup=[group]: Print all TypeIds of group." << std::endl;
  std::cout << "--PrintAttributes=[typeid]: Print all attributes of typeid." << std::endl;
  std::cout << "--PrintGlobals: Print the list of globals." << std::endl;
  if (!m_items.empty ())
    {
      std::cout << "User Arguments:" << std::endl;
      for (Items::const_iterator i = m_items.begin (); i != m_items.end (); ++i)
	{
	  std::cout << "    --" << (*i)->m_name << ": " << (*i)->m_help << std::endl;
	}
    }
  exit (0);
}

void
CommandLine::PrintGlobals (void) const
{
  for (GlobalValue::Iterator i = GlobalValue::Begin (); i != GlobalValue::End (); ++i)
    {
      std::cout << "    --" << (*i)->GetName () << "=[";
      Ptr<const AttributeChecker> checker = (*i)->GetChecker ();
      Attribute value = (*i)->GetValue ();
      std::cout << value.SerializeToString (checker) << "]:  "
		<< (*i)->GetHelp () << std::endl;      
    }
  exit (0);
}

void
CommandLine::PrintAttributes (std::string type) const
{
  TypeId tid;
  if (!TypeId::LookupByNameFailSafe (type, &tid))
    {
      NS_FATAL_ERROR ("Unknown type="<<type<<" in --PrintAttributes");
    }
  for (uint32_t i = 0; i < tid.GetAttributeListN (); ++i)
    {
      std::cout << "    --"<<tid.GetAttributeFullName (i)<<"=[";
      Ptr<const AttributeChecker> checker = tid.GetAttributeChecker (i);
      Attribute initial = tid.GetAttributeInitialValue (i);
      std::cout << initial.SerializeToString (checker) << "]:  "
		<< tid.GetAttributeHelp (i) << std::endl;
    }
  exit (0);
}


void
CommandLine::PrintGroup (std::string group) const
{
  for (uint32_t i = 0; i < TypeId::GetRegisteredN (); ++i)
    {
      TypeId tid = TypeId::GetRegistered (i);
      if (tid.GetGroupName () == group)
	{
	  std::cout << "    --PrintAttributes=" <<tid.GetName ()<<std::endl;
	}
    }
  exit (0);
}

void
CommandLine::PrintTypeIds (void) const
{
  for (uint32_t i = 0; i < TypeId::GetRegisteredN (); ++i)
    {
      TypeId tid = TypeId::GetRegistered (i);
      std::cout << "    --PrintAttributes=" <<tid.GetName ()<<std::endl;
    }
  exit (0);
}

void
CommandLine::PrintGroups (void) const
{
  std::list<std::string> groups;
  for (uint32_t i = 0; i < TypeId::GetRegisteredN (); ++i)
    {
      TypeId tid = TypeId::GetRegistered (i);
      std::string group = tid.GetGroupName ();
      if (group == "")
	{
	  continue;
	}
      bool found = false;
      for (std::list<std::string>::const_iterator j = groups.begin (); j != groups.end (); ++j)
	{
	  if (*j == group)
	    {
	      found = true;
	      break;
	    }
	}
      if (!found)
	{
	  groups.push_back (group);
	}
    }
  for (std::list<std::string>::const_iterator k = groups.begin (); k != groups.end (); ++k)
    {
      std::cout << "    --PrintGroup="<<*k<<std::endl;
    }
  exit (0);
}

void
CommandLine::HandleArgument (std::string name, std::string value) const
{
  NS_LOG_DEBUG ("Handle arg name="<<name<<" value="<<value);
  if (name == "PrintHelp")
    {
      // method below never returns.
      PrintHelp ();
    }
  if (name == "PrintGroups")
    {
      // method below never returns.
      PrintGroups ();
    }
  if (name == "PrintTypeIds")
    {
      // method below never returns.
      PrintTypeIds ();
    }
  if (name == "PrintGlobals")
    {
      // method below never returns.
      PrintGlobals ();
    }
  if (name == "PrintGroup")
    {
      // method below never returns.
      PrintGroup (value);
    }
  if (name == "PrintAttributes")
    {
      // method below never returns.
      PrintAttributes (value);
    }
  for (Items::const_iterator i = m_items.begin (); i != m_items.end (); ++i)
    {
      if ((*i)->m_name == name)
	{
	  if (!(*i)->Parse (value))
	    {
	      std::cerr << "Invalid argument value: "<<name<<"="<<value << std::endl;
	      return;
	    }
	  else
	    {
	      return;
	    }
	}
    }
  Config::SetGlobalFailSafe (name, String (value));
  Config::SetDefaultFailSafe (name, String (value));
  std::cerr << "Invalid command-line arguments: --"<<name<<"="<<value<<std::endl;
  PrintHelp ();
}

} // namespace ns3

#ifdef RUN_SELF_TESTS

#include "test.h"
#include <stdarg.h>

using namespace ns3;

class CommandLineTest : public Test
{
public:
  CommandLineTest ();
  virtual bool RunTests (void);
private:
  void Parse (const CommandLine &cmd, int n, ...);
};

CommandLineTest::CommandLineTest ()
  : Test ("CommandLine")
{}
void
CommandLineTest::Parse (const CommandLine &cmd, int n, ...)
{
  char **args = new char* [n+1];
  args[0] = "Test";
  va_list ap;
  va_start (ap, n);
  int i = 0;
  while (i < n)
    {
      char *arg = va_arg (ap, char *);
      args[i+1] = arg;
      i++;
    }
  int argc = n + 1;
  cmd.Parse (argc, args);
  delete [] args;
}
bool 
CommandLineTest::RunTests (void)
{
  bool result = true;

  bool myBool = false;
  uint32_t myUint32 = 10;
  std::string myStr = "MyStr";
  int32_t myInt32 = -1;
  CommandLine cmd;

  cmd.AddValue ("my-bool", "help", myBool);
  Parse (cmd, 1, "--my-bool=0");
  NS_TEST_ASSERT_EQUAL (myBool, false);
  Parse (cmd, 1, "--my-bool=1");
  NS_TEST_ASSERT_EQUAL (myBool, true);

  cmd.AddValue ("my-uint32", "help", myUint32);
  Parse (cmd, 2, "--my-bool=0", "--my-uint32=9");
  NS_TEST_ASSERT_EQUAL (myUint32, 9);

  cmd.AddValue ("my-str", "help", myStr);
  Parse (cmd, 2, "--my-bool=0", "--my-str=XX");
  NS_TEST_ASSERT_EQUAL (myStr, "XX");

  cmd.AddValue ("my-int32", "help", myInt32);
  Parse (cmd, 2, "--my-bool=0", "--my-int32=-3");
  NS_TEST_ASSERT_EQUAL (myInt32, -3);
  Parse (cmd, 2, "--my-bool=0", "--my-int32=+2");
  NS_TEST_ASSERT_EQUAL (myInt32, +2);

  return result;
}


static CommandLineTest g_cmdLineTest;

#endif /* RUN_SELF_TESTS */
