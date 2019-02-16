/*
                                   )
                                  (.)
                                  .|.
                                  | |
                              _.--| |--._
                           .-';  ;`-'& ; `&.
                          \   &  ;    &   &_/
                           |"""---...---"""|
                           \ | | | | | | | /
                            `---.|.|.|.---'

 * This file is generated by bake.lang.c for your convenience. Headers of
 * dependencies will automatically show up in this file. Include bake_config.h
 * in your main project file. Do not edit! */

#ifndef SANDBOX_BAKE_CONFIG_H
#define SANDBOX_BAKE_CONFIG_H

/* Headers of public dependencies */
#include <UI>
#include <Jobs>

/* Headers of private dependencies */
#ifdef SANDBOX_IMPL
/* No dependencies */
#endif

/* Convenience macro for exporting symbols */
#if SANDBOX_IMPL && defined _MSC_VER
#define SANDBOX_EXPORT __declspec(dllexport)
#elif SANDBOX_IMPL
#define SANDBOX_EXPORT __attribute__((__visibility__("default")))
#elif defined _MSC_VER
#define SANDBOX_EXPORT __declspec(dllimport)
#else
#define SANDBOX_EXPORT
#endif

#endif

