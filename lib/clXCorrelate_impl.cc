/* -*- c++ -*- */
/*                     GNU GENERAL PUBLIC LICENSE
 *                        Version 3, 29 June 2007
 *
 *  Copyright (C) 2007 Free Software Foundation, Inc. <http://fsf.org/>
 *  Everyone is permitted to copy and distribute verbatim copies
 *  of this license document, but changing it is not allowed.
 *
 *                             Preamble
 *
 *   The GNU General Public License is a free, copyleft license for
 * software and other kinds of works.
 *
 *   The licenses for most software and other practical works are designed
 * to take away your freedom to share and change the works.  By contrast,
 * the GNU General Public License is intended to guarantee your freedom to
 * share and change all versions of a program--to make sure it remains free
 * software for all its users.  We, the Free Software Foundation, use the
 * GNU General Public License for most of our software; it applies also to
 * any other work released this way by its authors.  You can apply it to
 * your programs, too.
 *
 *   When we speak of free software, we are referring to freedom, not
 * price.  Our General Public Licenses are designed to make sure that you
 * have the freedom to distribute copies of free software (and charge for
 * them if you wish), that you receive source code or can get it if you
 * want it, that you can change the software or use pieces of it in new
 * free programs, and that you know you can do these things.
 *
 *   To protect your rights, we need to prevent others from denying you
 * these rights or asking you to surrender the rights.  Therefore, you have
 * certain responsibilities if you distribute copies of the software, or if
 * you modify it: responsibilities to respect the freedom of others.
 *
 *   For example, if you distribute copies of such a program, whether
 * gratis or for a fee, you must pass on to the recipients the same
 * freedoms that you received.  You must make sure that they, too, receive
 * or can get the source code.  And you must show them these terms so they
 * know their rights.
 *
 *   Developers that use the GNU GPL protect your rights with two steps:
 * (1) assert copyright on the software, and (2) offer you this License
 * giving you legal permission to copy, distribute and/or modify it.
 *
 *   For the developers' and authors' protection, the GPL clearly explains
 * that there is no warranty for this free software.  For both users' and
 * authors' sake, the GPL requires that modified versions be marked as
 * changed, so that their problems will not be attributed erroneously to
 * authors of previous versions.
 *
 *   Some devices are designed to deny users access to install or run
 * modified versions of the software inside them, although the manufacturer
 * can do so.  This is fundamentally incompatible with the aim of
 * protecting users' freedom to change the software.  The systematic
 * pattern of such abuse occurs in the area of products for individuals to
 * use, which is precisely where it is most unacceptable.  Therefore, we
 * have designed this version of the GPL to prohibit the practice for those
 * products.  If such problems arise substantially in other domains, we
 * stand ready to extend this provision to those domains in future versions
 * of the GPL, as needed to protect the freedom of users.
 *
 *   Finally, every program is threatened constantly by software patents.
 * States should not allow patents to restrict development and use of
 * software on general-purpose computers, but in those that do, we wish to
 * avoid the special danger that patents applied to a free program could
 * make it effectively proprietary.  To prevent this, the GPL assures that
 * patents cannot be used to render the program non-free.
 *
 *   The precise terms and conditions for copying, distribution and
 * modification follow.
 *
 *                        TERMS AND CONDITIONS
 *
 *   0. Definitions.
 *
 *   "This License" refers to version 3 of the GNU General Public License.
 *
 *   "Copyright" also means copyright-like laws that apply to other kinds of
 * works, such as semiconductor masks.
 *
 *   "The Program" refers to any copyrightable work licensed under this
 * License.  Each licensee is addressed as "you".  "Licensees" and
 * "recipients" may be individuals or organizations.
 *
 *   To "modify" a work means to copy from or adapt all or part of the work
 * in a fashion requiring copyright permission, other than the making of an
 * exact copy.  The resulting work is called a "modified version" of the
 * earlier work or a work "based on" the earlier work.
 *
 *   A "covered work" means either the unmodified Program or a work based
 * on the Program.
 *
 *   To "propagate" a work means to do anything with it that, without
 * permission, would make you directly or secondarily liable for
 * infringement under applicable copyright law, except executing it on a
 * computer or modifying a private copy.  Propagation includes copying,
 * distribution (with or without modification), making available to the
 * public, and in some countries other activities as well.
 *
 *   To "convey" a work means any kind of propagation that enables other
 * parties to make or receive copies.  Mere interaction with a user through
 * a computer network, with no transfer of a copy, is not conveying.
 *
 *   An interactive user interface displays "Appropriate Legal Notices"
 * to the extent that it includes a convenient and prominently visible
 * feature that (1) displays an appropriate copyright notice, and (2)
 * tells the user that there is no warranty for the work (except to the
 * extent that warranties are provided), that licensees may convey the
 * work under this License, and how to view a copy of this License.  If
 * the interface presents a list of user commands or options, such as a
 * menu, a prominent item in the list meets this criterion.
 *
 *   1. Source Code.
 *
 *   The "source code" for a work means the preferred form of the work
 * for making modifications to it.  "Object code" means any non-source
 * form of a work.
 *
 *   A "Standard Interface" means an interface that either is an official
 * standard defined by a recognized standards body, or, in the case of
 * interfaces specified for a particular programming language, one that
 * is widely used among developers working in that language.
 *
 *   The "System Libraries" of an executable work include anything, other
 * than the work as a whole, that (a) is included in the normal form of
 * packaging a Major Component, but which is not part of that Major
 * Component, and (b) serves only to enable use of the work with that
 * Major Component, or to implement a Standard Interface for which an
 * implementation is available to the public in source code form.  A
 * "Major Component", in this context, means a major essential component
 * (kernel, window system, and so on) of the specific operating system
 * (if any) on which the executable work runs, or a compiler used to
 * produce the work, or an object code interpreter used to run it.
 *
 *   The "Corresponding Source" for a work in object code form means all
 * the source code needed to generate, install, and (for an executable
 * work) run the object code and to modify the work, including scripts to
 * control those activities.  However, it does not include the work's
 * System Libraries, or general-purpose tools or generally available free
 * programs which are used unmodified in performing those activities but
 * which are not part of the work.  For example, Corresponding Source
 * includes interface definition files associated with source files for
 * the work, and the source code for shared libraries and dynamically
 * linked subprograms that the work is specifically designed to require,
 * such as by intimate data communication or control flow between those
 * subprograms and other parts of the work.
 *
 *   The Corresponding Source need not include anything that users
 * can regenerate automatically from other parts of the Corresponding
 * Source.
 *
 *   The Corresponding Source for a work in source code form is that
 * same work.
 *
 *   2. Basic Permissions.
 *
 *   All rights granted under this License are granted for the term of
 * copyright on the Program, and are irrevocable provided the stated
 * conditions are met.  This License explicitly affirms your unlimited
 * permission to run the unmodified Program.  The output from running a
 * covered work is covered by this License only if the output, given its
 * content, constitutes a covered work.  This License acknowledges your
 * rights of fair use or other equivalent, as provided by copyright law.
 *
 *   You may make, run and propagate covered works that you do not
 * convey, without conditions so long as your license otherwise remains
 * in force.  You may convey covered works to others for the sole purpose
 * of having them make modifications exclusively for you, or provide you
 * with facilities for running those works, provided that you comply with
 * the terms of this License in conveying all material for which you do
 * not control copyright.  Those thus making or running the covered works
 * for you must do so exclusively on your behalf, under your direction
 * and control, on terms that prohibit them from making any copies of
 * your copyrighted material outside their relationship with you.
 *
 *   Conveying under any other circumstances is permitted solely under
 * the conditions stated below.  Sublicensing is not allowed; section 10
 * makes it unnecessary.
 *
 *   3. Protecting Users' Legal Rights From Anti-Circumvention Law.
 *
 *   No covered work shall be deemed part of an effective technological
 * measure under any applicable law fulfilling obligations under article
 * 11 of the WIPO copyright treaty adopted on 20 December 1996, or
 * similar laws prohibiting or restricting circumvention of such
 * measures.
 *
 *   When you convey a covered work, you waive any legal power to forbid
 * circumvention of technological measures to the extent such circumvention
 * is effected by exercising rights under this License with respect to
 * the covered work, and you disclaim any intention to limit operation or
 * modification of the work as a means of enforcing, against the work's
 * users, your or third parties' legal rights to forbid circumvention of
 * technological measures.
 *
 *   4. Conveying Verbatim Copies.
 *
 *   You may convey verbatim copies of the Program's source code as you
 * receive it, in any medium, provided that you conspicuously and
 * appropriately publish on each copy an appropriate copyright notice;
 * keep intact all notices stating that this License and any
 * non-permissive terms added in accord with section 7 apply to the code;
 * keep intact all notices of the absence of any warranty; and give all
 * recipients a copy of this License along with the Program.
 *
 *   You may charge any price or no price for each copy that you convey,
 * and you may offer support or warranty protection for a fee.
 *
 *   5. Conveying Modified Source Versions.
 *
 *   You may convey a work based on the Program, or the modifications to
 * produce it from the Program, in the form of source code under the
 * terms of section 4, provided that you also meet all of these conditions:
 *
 *     a) The work must carry prominent notices stating that you modified
 *     it, and giving a relevant date.
 *
 *     b) The work must carry prominent notices stating that it is
 *     released under this License and any conditions added under section
 *     7.  This requirement modifies the requirement in section 4 to
 *     "keep intact all notices".
 *
 *     c) You must license the entire work, as a whole, under this
 *     License to anyone who comes into possession of a copy.  This
 *     License will therefore apply, along with any applicable section 7
 *     additional terms, to the whole of the work, and all its parts,
 *     regardless of how they are packaged.  This License gives no
 *     permission to license the work in any other way, but it does not
 *     invalidate such permission if you have separately received it.
 *
 *     d) If the work has interactive user interfaces, each must display
 *     Appropriate Legal Notices; however, if the Program has interactive
 *     interfaces that do not display Appropriate Legal Notices, your
 *     work need not make them do so.
 *
 *   A compilation of a covered work with other separate and independent
 * works, which are not by their nature extensions of the covered work,
 * and which are not combined with it such as to form a larger program,
 * in or on a volume of a storage or distribution medium, is called an
 * "aggregate" if the compilation and its resulting copyright are not
 * used to limit the access or legal rights of the compilation's users
 * beyond what the individual works permit.  Inclusion of a covered work
 * in an aggregate does not cause this License to apply to the other
 * parts of the aggregate.
 *
 *   6. Conveying Non-Source Forms.
 *
 *   You may convey a covered work in object code form under the terms
 * of sections 4 and 5, provided that you also convey the
 * machine-readable Corresponding Source under the terms of this License,
 * in one of these ways:
 *
 *     a) Convey the object code in, or embodied in, a physical product
 *     (including a physical distribution medium), accompanied by the
 *     Corresponding Source fixed on a durable physical medium
 *     customarily used for software interchange.
 *
 *     b) Convey the object code in, or embodied in, a physical product
 *     (including a physical distribution medium), accompanied by a
 *     written offer, valid for at least three years and valid for as
 *     long as you offer spare parts or customer support for that product
 *     model, to give anyone who possesses the object code either (1) a
 *     copy of the Corresponding Source for all the software in the
 *     product that is covered by this License, on a durable physical
 *     medium customarily used for software interchange, for a price no
 *     more than your reasonable cost of physically performing this
 *     conveying of source, or (2) access to copy the
 *     Corresponding Source from a network server at no charge.
 *
 *     c) Convey individual copies of the object code with a copy of the
 *     written offer to provide the Corresponding Source.  This
 *     alternative is allowed only occasionally and noncommercially, and
 *     only if you received the object code with such an offer, in accord
 *     with subsection 6b.
 *
 *     d) Convey the object code by offering access from a designated
 *     place (gratis or for a charge), and offer equivalent access to the
 *     Corresponding Source in the same way through the same place at no
 *     further charge.  You need not require recipients to copy the
 *     Corresponding Source along with the object code.  If the place to
 *     copy the object code is a network server, the Corresponding Source
 *     may be on a different server (operated by you or a third party)
 *     that supports equivalent copying facilities, provided you maintain
 *     clear directions next to the object code saying where to find the
 *     Corresponding Source.  Regardless of what server hosts the
 *     Corresponding Source, you remain obligated to ensure that it is
 *     available for as long as needed to satisfy these requirements.
 *
 *     e) Convey the object code using peer-to-peer transmission, provided
 *     you inform other peers where the object code and Corresponding
 *     Source of the work are being offered to the general public at no
 *     charge under subsection 6d.
 *
 *   A separable portion of the object code, whose source code is excluded
 * from the Corresponding Source as a System Library, need not be
 * included in conveying the object code work.
 *
 *   A "User Product" is either (1) a "consumer product", which means any
 * tangible personal property which is normally used for personal, family,
 * or household purposes, or (2) anything designed or sold for incorporation
 * into a dwelling.  In determining whether a product is a consumer product,
 * doubtful cases shall be resolved in favor of coverage.  For a particular
 * product received by a particular user, "normally used" refers to a
 * typical or common use of that class of product, regardless of the status
 * of the particular user or of the way in which the particular user
 * actually uses, or expects or is expected to use, the product.  A product
 * is a consumer product regardless of whether the product has substantial
 * commercial, industrial or non-consumer uses, unless such uses represent
 * the only significant mode of use of the product.
 *
 *   "Installation Information" for a User Product means any methods,
 * procedures, authorization keys, or other information required to install
 * and execute modified versions of a covered work in that User Product from
 * a modified version of its Corresponding Source.  The information must
 * suffice to ensure that the continued functioning of the modified object
 * code is in no case prevented or interfered with solely because
 * modification has been made.
 *
 *   If you convey an object code work under this section in, or with, or
 * specifically for use in, a User Product, and the conveying occurs as
 * part of a transaction in which the right of possession and use of the
 * User Product is transferred to the recipient in perpetuity or for a
 * fixed term (regardless of how the transaction is characterized), the
 * Corresponding Source conveyed under this section must be accompanied
 * by the Installation Information.  But this requirement does not apply
 * if neither you nor any third party retains the ability to install
 * modified object code on the User Product (for example, the work has
 * been installed in ROM).
 *
 *   The requirement to provide Installation Information does not include a
 * requirement to continue to provide support service, warranty, or updates
 * for a work that has been modified or installed by the recipient, or for
 * the User Product in which it has been modified or installed.  Access to a
 * network may be denied when the modification itself materially and
 * adversely affects the operation of the network or violates the rules and
 * protocols for communication across the network.
 *
 *   Corresponding Source conveyed, and Installation Information provided,
 * in accord with this section must be in a format that is publicly
 * documented (and with an implementation available to the public in
 * source code form), and must require no special password or key for
 * unpacking, reading or copying.
 *
 *   7. Additional Terms.
 *
 *   "Additional permissions" are terms that supplement the terms of this
 * License by making exceptions from one or more of its conditions.
 * Additional permissions that are applicable to the entire Program shall
 * be treated as though they were included in this License, to the extent
 * that they are valid under applicable law.  If additional permissions
 * apply only to part of the Program, that part may be used separately
 * under those permissions, but the entire Program remains governed by
 * this License without regard to the additional permissions.
 *
 *   When you convey a copy of a covered work, you may at your option
 * remove any additional permissions from that copy, or from any part of
 * it.  (Additional permissions may be written to require their own
 * removal in certain cases when you modify the work.)  You may place
 * additional permissions on material, added by you to a covered work,
 * for which you have or can give appropriate copyright permission.
 *
 *   Notwithstanding any other provision of this License, for material you
 * add to a covered work, you may (if authorized by the copyright holders of
 * that material) supplement the terms of this License with terms:
 *
 *     a) Disclaiming warranty or limiting liability differently from the
 *     terms of sections 15 and 16 of this License; or
 *
 *     b) Requiring preservation of specified reasonable legal notices or
 *     author attributions in that material or in the Appropriate Legal
 *     Notices displayed by works containing it; or
 *
 *     c) Prohibiting misrepresentation of the origin of that material, or
 *     requiring that modified versions of such material be marked in
 *     reasonable ways as different from the original version; or
 *
 *     d) Limiting the use for publicity purposes of names of licensors or
 *     authors of the material; or
 *
 *     e) Declining to grant rights under trademark law for use of some
 *     trade names, trademarks, or service marks; or
 *
 *     f) Requiring indemnification of licensors and authors of that
 *     material by anyone who conveys the material (or modified versions of
 *     it) with contractual assumptions of liability to the recipient, for
 *     any liability that these contractual assumptions directly impose on
 *     those licensors and authors.
 *
 *   All other non-permissive additional terms are considered "further
 * restrictions" within the meaning of section 10.  If the Program as you
 * received it, or any part of it, contains a notice stating that it is
 * governed by this License along with a term that is a further
 * restriction, you may remove that term.  If a license document contains
 * a further restriction but permits relicensing or conveying under this
 * License, you may add to a covered work material governed by the terms
 * of that license document, provided that the further restriction does
 * not survive such relicensing or conveying.
 *
 *   If you add terms to a covered work in accord with this section, you
 * must place, in the relevant source files, a statement of the
 * additional terms that apply to those files, or a notice indicating
 * where to find the applicable terms.
 *
 *   Additional terms, permissive or non-permissive, may be stated in the
 * form of a separately written license, or stated as exceptions;
 * the above requirements apply either way.
 *
 *   8. Termination.
 *
 *   You may not propagate or modify a covered work except as expressly
 * provided under this License.  Any attempt otherwise to propagate or
 * modify it is void, and will automatically terminate your rights under
 * this License (including any patent licenses granted under the third
 * paragraph of section 11).
 *
 *   However, if you cease all violation of this License, then your
 * license from a particular copyright holder is reinstated (a)
 * provisionally, unless and until the copyright holder explicitly and
 * finally terminates your license, and (b) permanently, if the copyright
 * holder fails to notify you of the violation by some reasonable means
 * prior to 60 days after the cessation.
 *
 *   Moreover, your license from a particular copyright holder is
 * reinstated permanently if the copyright holder notifies you of the
 * violation by some reasonable means, this is the first time you have
 * received notice of violation of this License (for any work) from that
 * copyright holder, and you cure the violation prior to 30 days after
 * your receipt of the notice.
 *
 *   Termination of your rights under this section does not terminate the
 * licenses of parties who have received copies or rights from you under
 * this License.  If your rights have been terminated and not permanently
 * reinstated, you do not qualify to receive new licenses for the same
 * material under section 10.
 *
 *   9. Acceptance Not Required for Having Copies.
 *
 *   You are not required to accept this License in order to receive or
 * run a copy of the Program.  Ancillary propagation of a covered work
 * occurring solely as a consequence of using peer-to-peer transmission
 * to receive a copy likewise does not require acceptance.  However,
 * nothing other than this License grants you permission to propagate or
 * modify any covered work.  These actions infringe copyright if you do
 * not accept this License.  Therefore, by modifying or propagating a
 * covered work, you indicate your acceptance of this License to do so.
 *
 *   10. Automatic Licensing of Downstream Recipients.
 *
 *   Each time you convey a covered work, the recipient automatically
 * receives a license from the original licensors, to run, modify and
 * propagate that work, subject to this License.  You are not responsible
 * for enforcing compliance by third parties with this License.
 *
 *   An "entity transaction" is a transaction transferring control of an
 * organization, or substantially all assets of one, or subdividing an
 * organization, or merging organizations.  If propagation of a covered
 * work results from an entity transaction, each party to that
 * transaction who receives a copy of the work also receives whatever
 * licenses to the work the party's predecessor in interest had or could
 * give under the previous paragraph, plus a right to possession of the
 * Corresponding Source of the work from the predecessor in interest, if
 * the predecessor has it or can get it with reasonable efforts.
 *
 *   You may not impose any further restrictions on the exercise of the
 * rights granted or affirmed under this License.  For example, you may
 * not impose a license fee, royalty, or other charge for exercise of
 * rights granted under this License, and you may not initiate litigation
 * (including a cross-claim or counterclaim in a lawsuit) alleging that
 * any patent claim is infringed by making, using, selling, offering for
 * sale, or importing the Program or any portion of it.
 *
 *   11. Patents.
 *
 *   A "contributor" is a copyright holder who authorizes use under this
 * License of the Program or a work on which the Program is based.  The
 * work thus licensed is called the contributor's "contributor version".
 *
 *   A contributor's "essential patent claims" are all patent claims
 * owned or controlled by the contributor, whether already acquired or
 * hereafter acquired, that would be infringed by some manner, permitted
 * by this License, of making, using, or selling its contributor version,
 * but do not include claims that would be infringed only as a
 * consequence of further modification of the contributor version.  For
 * purposes of this definition, "control" includes the right to grant
 * patent sublicenses in a manner consistent with the requirements of
 * this License.
 *
 *   Each contributor grants you a non-exclusive, worldwide, royalty-free
 * patent license under the contributor's essential patent claims, to
 * make, use, sell, offer for sale, import and otherwise run, modify and
 * propagate the contents of its contributor version.
 *
 *   In the following three paragraphs, a "patent license" is any express
 * agreement or commitment, however denominated, not to enforce a patent
 * (such as an express permission to practice a patent or covenant not to
 * sue for patent infringement).  To "grant" such a patent license to a
 * party means to make such an agreement or commitment not to enforce a
 * patent against the party.
 *
 *   If you convey a covered work, knowingly relying on a patent license,
 * and the Corresponding Source of the work is not available for anyone
 * to copy, free of charge and under the terms of this License, through a
 * publicly available network server or other readily accessible means,
 * then you must either (1) cause the Corresponding Source to be so
 * available, or (2) arrange to deprive yourself of the benefit of the
 * patent license for this particular work, or (3) arrange, in a manner
 * consistent with the requirements of this License, to extend the patent
 * license to downstream recipients.  "Knowingly relying" means you have
 * actual knowledge that, but for the patent license, your conveying the
 * covered work in a country, or your recipient's use of the covered work
 * in a country, would infringe one or more identifiable patents in that
 * country that you have reason to believe are valid.
 *
 *   If, pursuant to or in connection with a single transaction or
 * arrangement, you convey, or propagate by procuring conveyance of, a
 * covered work, and grant a patent license to some of the parties
 * receiving the covered work authorizing them to use, propagate, modify
 * or convey a specific copy of the covered work, then the patent license
 * you grant is automatically extended to all recipients of the covered
 * work and works based on it.
 *
 *   A patent license is "discriminatory" if it does not include within
 * the scope of its coverage, prohibits the exercise of, or is
 * conditioned on the non-exercise of one or more of the rights that are
 * specifically granted under this License.  You may not convey a covered
 * work if you are a party to an arrangement with a third party that is
 * in the business of distributing software, under which you make payment
 * to the third party based on the extent of your activity of conveying
 * the work, and under which the third party grants, to any of the
 * parties who would receive the covered work from you, a discriminatory
 * patent license (a) in connection with copies of the covered work
 * conveyed by you (or copies made from those copies), or (b) primarily
 * for and in connection with specific products or compilations that
 * contain the covered work, unless you entered into that arrangement,
 * or that patent license was granted, prior to 28 March 2007.
 *
 *   Nothing in this License shall be construed as excluding or limiting
 * any implied license or other defenses to infringement that may
 * otherwise be available to you under applicable patent law.
 *
 *   12. No Surrender of Others' Freedom.
 *
 *   If conditions are imposed on you (whether by court order, agreement or
 * otherwise) that contradict the conditions of this License, they do not
 * excuse you from the conditions of this License.  If you cannot convey a
 * covered work so as to satisfy simultaneously your obligations under this
 * License and any other pertinent obligations, then as a consequence you may
 * not convey it at all.  For example, if you agree to terms that obligate you
 * to collect a royalty for further conveying from those to whom you convey
 * the Program, the only way you could satisfy both those terms and this
 * License would be to refrain entirely from conveying the Program.
 *
 *   13. Use with the GNU Affero General Public License.
 *
 *   Notwithstanding any other provision of this License, you have
 * permission to link or combine any covered work with a work licensed
 * under version 3 of the GNU Affero General Public License into a single
 * combined work, and to convey the resulting work.  The terms of this
 * License will continue to apply to the part which is the covered work,
 * but the special requirements of the GNU Affero General Public License,
 * section 13, concerning interaction through a network will apply to the
 * combination as such.
 *
 *   14. Revised Versions of this License.
 *
 *   The Free Software Foundation may publish revised and/or new versions of
 * the GNU General Public License from time to time.  Such new versions will
 * be similar in spirit to the present version, but may differ in detail to
 * address new problems or concerns.
 *
 *   Each version is given a distinguishing version number.  If the
 * Program specifies that a certain numbered version of the GNU General
 * Public License "or any later version" applies to it, you have the
 * option of following the terms and conditions either of that numbered
 * version or of any later version published by the Free Software
 * Foundation.  If the Program does not specify a version number of the
 * GNU General Public License, you may choose any version ever published
 * by the Free Software Foundation.
 *
 *   If the Program specifies that a proxy can decide which future
 * versions of the GNU General Public License can be used, that proxy's
 * public statement of acceptance of a version permanently authorizes you
 * to choose that version for the Program.
 *
 *   Later license versions may give you additional or different
 * permissions.  However, no additional obligations are imposed on any
 * author or copyright holder as a result of your choosing to follow a
 * later version.
 *
 *   15. Disclaimer of Warranty.
 *
 *   THERE IS NO WARRANTY FOR THE PROGRAM, TO THE EXTENT PERMITTED BY
 * APPLICABLE LAW.  EXCEPT WHEN OTHERWISE STATED IN WRITING THE COPYRIGHT
 * HOLDERS AND/OR OTHER PARTIES PROVIDE THE PROGRAM "AS IS" WITHOUT WARRANTY
 * OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE.  THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE OF THE PROGRAM
 * IS WITH YOU.  SHOULD THE PROGRAM PROVE DEFECTIVE, YOU ASSUME THE COST OF
 * ALL NECESSARY SERVICING, REPAIR OR CORRECTION.
 *
 *   16. Limitation of Liability.
 *
 *   IN NO EVENT UNLESS REQUIRED BY APPLICABLE LAW OR AGREED TO IN WRITING
 * WILL ANY COPYRIGHT HOLDER, OR ANY OTHER PARTY WHO MODIFIES AND/OR CONVEYS
 * THE PROGRAM AS PERMITTED ABOVE, BE LIABLE TO YOU FOR DAMAGES, INCLUDING ANY
 * GENERAL, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES ARISING OUT OF THE
 * USE OR INABILITY TO USE THE PROGRAM (INCLUDING BUT NOT LIMITED TO LOSS OF
 * DATA OR DATA BEING RENDERED INACCURATE OR LOSSES SUSTAINED BY YOU OR THIRD
 * PARTIES OR A FAILURE OF THE PROGRAM TO OPERATE WITH ANY OTHER PROGRAMS),
 * EVEN IF SUCH HOLDER OR OTHER PARTY HAS BEEN ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGES.
 *
 *   17. Interpretation of Sections 15 and 16.
 *
 *   If the disclaimer of warranty and limitation of liability provided
 * above cannot be given local legal effect according to their terms,
 * reviewing courts shall apply local law that most closely approximates
 * an absolute waiver of all civil liability in connection with the
 * Program, unless a warranty or assumption of liability accompanies a
 * copy of the Program in return for a fee.
 *
 *                      END OF TERMS AND CONDITIONS
 *
 *             How to Apply These Terms to Your New Programs
 *
 *   If you develop a new program, and you want it to be of the greatest
 * possible use to the public, the best way to achieve this is to make it
 * free software which everyone can redistribute and change under these terms.
 *
 *   To do so, attach the following notices to the program.  It is safest
 * to attach them to the start of each source file to most effectively
 * state the exclusion of warranty; and each file should have at least
 * the "copyright" line and a pointer to where the full notice is found.
 *
 *     {one line to give the program's name and a brief idea of what it does.}
 *     Copyright (C) {year}  {name of author}
 *
 *     This program is free software: you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation, either version 3 of the License, or
 *     (at your option) any later version.
 *
 *     This program is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 *
 *     You should have received a copy of the GNU General Public License
 *     along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Also add information on how to contact you by electronic and paper mail.
 *
 *   If the program does terminal interaction, make it output a short
 * notice like this when it starts in an interactive mode:
 *
 *     {project}  Copyright (C) {year}  {fullname}
 *     This program comes with ABSOLUTELY NO WARRANTY; for details type `show w'.
 *     This is free software, and you are welcome to redistribute it
 *     under certain conditions; type `show c' for details.
 *
 * The hypothetical commands `show w' and `show c' should show the appropriate
 * parts of the General Public License.  Of course, your program's commands
 * might be different; for a GUI interface, you would use an "about box".
 *
 *   You should also get your employer (if you work as a programmer) or school,
 * if any, to sign a "copyright disclaimer" for the program, if necessary.
 * For more information on this, and how to apply and follow the GNU GPL, see
 * <http://www.gnu.org/licenses/>.
 *
 *   The GNU General Public License does not permit incorporating your program
 * into proprietary programs.  If your program is a subroutine library, you
 * may consider it more useful to permit linking proprietary applications with
 * the library.  If this is what you want to do, use the GNU Lesser General
 * Public License instead of this License.  But first, please read
 * <http://www.gnu.org/philosophy/why-not-lgpl.html>.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include "clXCorrelate_impl.h"

namespace gr {
namespace clenabled {

clXCorrelate::sptr
clXCorrelate::make(int openCLPlatformType,int devSelector,int platformId, int devId, bool setDebug,
		int num_inputs, int signal_length, int data_type, int data_size, int max_search_index, int decim_frames, bool async)
{
	return gnuradio::get_initial_sptr
			(new clXCorrelate_impl(openCLPlatformType,devSelector,platformId, devId, setDebug,
					num_inputs, signal_length, data_type, data_size, max_search_index, decim_frames, async));
}


/*
 * The private constructor
 */
clXCorrelate_impl::clXCorrelate_impl(int openCLPlatformType,int devSelector,int platformId, int devId, bool setDebug,
		int num_inputs, int signal_length, int data_type, int data_size, int max_search_index, int decim_frames, bool async)
: gr::sync_block("clXCorrelate",
		gr::io_signature::make(2, num_inputs, data_size),
		gr::io_signature::make(0, 0, 0)),
		GRCLBase(data_type, data_size,openCLPlatformType,devSelector,platformId,devId,setDebug),
		d_num_inputs(num_inputs), d_signal_length(signal_length), d_data_type(data_type),
		d_data_size(data_size), d_decim_frames(decim_frames), max_shift(max_search_index), d_async(async), cur_frame_counter(1)
{
	if (data_size == 0) {
		// Had to wait to get insider here for access to d_logger
		GR_LOG_ERROR(d_logger, "Unknown data type.");
		exit(1);
	}

	if ((d_signal_length % 2) > 0) {
		GR_LOG_ERROR(d_logger, "Signal length must be a multiple of 2.");
		exit(1);
	}

	if ((max_shift % 2) > 0) {
		GR_LOG_ERROR(d_logger, "max shift must be a multiple of 2.");
		exit(1);
	}

	signal_byte_size = d_signal_length * data_size;

	if (max_search_index > 0) {
		max_shift = max_search_index;
	}
	else {
		max_shift = (int)(0.7 * (float)d_signal_length);
		if ((max_shift % 2) > 0) {
			max_shift += 1;
		}
	}

	// For the max kernel to work correctly, max_shift needs to be a power of 2 boundary.
	float p2 = log2(max_shift);

	int new_max_shift = (int)pow(2,ceil(p2));
	if (new_max_shift != max_shift) {
		std::cout << "Adjusting max shift to " << new_max_shift << " for power-of-2 boundary" << std::endl;
		max_shift = new_max_shift;
	}

	max_shift_2 = 2 * max_shift;

	// BuildKernel needs to be called early or the max workgroup stuff isn't available yet.
	buildKernel();
	buildCCMagKernel();
	buildF32SquaredKernel();
	buildFindMaxKernel();

	// We have 4 different kernels, for the calcs we need for the max kernel, we specifically
	// want the max sizes for that kernel.
	maxWorkGroupSize = find_max_kernel->getWorkGroupInfo<CL_KERNEL_WORK_GROUP_SIZE>(devices[0]);
	preferredWorkGroupSizeMultiple = find_max_kernel->getWorkGroupInfo<CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE>(devices[0]);

	localWGSize = cl::NullRange;
	/*
	if (contextType!=CL_DEVICE_TYPE_CPU) {
		if (d_signal_length % preferredWorkGroupSizeMultiple == 0) {
			// for some reason problems start to happen when we're no longer using constant memory
			localWGSize=cl::NDRange(preferredWorkGroupSizeMultiple);
		}
	}
	*/
	// We do need to watch workgroup sizes for the final max() function.
	// We're going to have max_shift * 2 entries to process.
	// We'll need to optimize based on maxWorkGroupSize and preferredWorkGroupSizeMultiple to process
	// as much of the max calculations on the GPU as possible, so we're going to want workgroups to be
	// as big as possible to have as few workgroups as possible.
	max_calc_workgroup_size = maxWorkGroupSize;

	if (max_shift_2 > maxWorkGroupSize) {
		bool found_match = false;
		int cur_size = maxWorkGroupSize;

		// Let's start with a workgroup size of max size and bump it down from there at optimal increments
		// till we find an even match.
		while (!found_match and cur_size > 0) {
			if ((max_shift_2 % cur_size) == 0) {
				found_match = true;
			}
			else {
				cur_size -= preferredWorkGroupSizeMultiple;
			}
		}

		if (found_match) {
			num_max_calc_workgroups = max_shift_2 / cur_size;
			max_calc_workgroup_size = cur_size;
		}
		else {
			// We'll have to go sub-optimal.  Just find an evenly
			// divisible power of 2 that's less than max workgroup size.
			cur_size = max_shift_2;
			found_match = false;

			while (!found_match and cur_size > 0) {
				if (cur_size < maxWorkGroupSize) {
					found_match = true;
				}
				else {
					cur_size /= 2;
				}
			}

			// max_shift_2 will always be a power of 2 so we'll eventually find one.
			num_max_calc_workgroups = max_shift_2 / cur_size;
			max_calc_workgroup_size = cur_size;
		}
	}
	else {
		num_max_calc_workgroups = 1;
		max_calc_workgroup_size = max_shift_2;
	}

	maxCalcWGSize = cl::NDRange(max_calc_workgroup_size);

	setBufferLength();

	// 2nd queue for parallel GPU tasks.
	queue2 = new cl::CommandQueue(*context, devices[devIndex], 0);

	if (d_async) {
		if (d_data_type == DTYPE_COMPLEX) {
			d_input_buffer_complex = new gr_complex[d_signal_length * d_num_inputs];
		}
		else {
			d_input_buffer_real = new float[d_signal_length * d_num_inputs];
		}

		proc_thread = new boost::thread(boost::bind(&clXCorrelate_impl::runThread, this));
	}
	// signal_length represents how long a signal in terms of chunk of samples
	// that we want to analyze for correlation.
	gr::block::set_output_multiple(d_signal_length);
	message_port_register_out(pmt::mp("corr"));
}

void clXCorrelate_impl::buildKernel() {
	std::string srcStdStr="";
	std::string fnName = "XCorrelate";

	// Use #defines to not have to pass them in as params since they won't change
	srcStdStr += "#define max_shift " + std::to_string(max_shift) + "\n";
	srcStdStr += "#define d_signal_length " + std::to_string(d_signal_length) + "\n\n";

	srcStdStr += "__kernel void XCorrelate(__global float * restrict ref_mag_buffer, __global float * restrict mag_buffer, \n";
	srcStdStr += "							__global float * restrict xx_buffer, __global float * restrict yy_buffer,\n";
	srcStdStr += "							__global float * restrict correlation_factors) {\n";
	// To get the kernel to handle both fwd and backward shifts, our NDRange will be 2 * max_shift.
	// So we need to pull it back in here.
	srcStdStr += "	int g_id =  get_global_id(0);\n";
	srcStdStr += "	int current_shift =  g_id - max_shift;\n";
	srcStdStr += "	int ref_start =  (current_shift>=0)?current_shift:-current_shift;\n";
	srcStdStr += "	float sum_xy = 0;\n";
	srcStdStr += "	float sum_x2 = 0;\n";
	srcStdStr += "	float sum_y2 = 0;\n";
	srcStdStr += "	float cur_corr;\n";
	srcStdStr += "	int calc_len = d_signal_length - ref_start;;\n";
	srcStdStr += "	float denom;\n";

	// Calculate sum(x*y) / sqrt(sum(x^2) * sum(y^2))

	// Visualization for this shift with y forward
	// x:     0   1   2   3   4   5
	// y:         0   1   2   3   4
	// Visualization for this shift with x signal forward
	// x:             0   1   2   3
	// y:     0   1   2   3   4   5

	srcStdStr += "if (current_shift > 0) {\n";
	srcStdStr += "	for (int i=0;i<calc_len;i++) {\n";
	srcStdStr += "		sum_xy += ref_mag_buffer[ref_start+i] * mag_buffer[i];\n";
	srcStdStr += "		sum_x2 += xx_buffer[ref_start+i];\n";
	srcStdStr += "		sum_y2 += yy_buffer[i];\n";
	srcStdStr += "	}\n";
	srcStdStr += "}\n";
	srcStdStr += "else {\n";
	srcStdStr += "	for (int i=0;i<calc_len;i++) {\n";
	srcStdStr += "		sum_xy += ref_mag_buffer[i] * mag_buffer[ref_start+i];\n";
	srcStdStr += "		sum_x2 += xx_buffer[i];\n";
	srcStdStr += "		sum_y2 += yy_buffer[ref_start+i];\n";
	srcStdStr += "	}\n";
	srcStdStr += "}\n";

	srcStdStr += "	denom = sum_x2 * sum_y2;\n";

	srcStdStr += "	if (denom != 0.0) {\n";
	srcStdStr += "		cur_corr = sum_xy / sqrt(sum_x2 * sum_y2);\n";
	srcStdStr += "	}\n";
	srcStdStr += "	else {\n";
	srcStdStr += "		cur_corr = -2.0;\n";
	srcStdStr += "	}\n";

	srcStdStr += "	correlation_factors[g_id] = cur_corr;\n";
	srcStdStr += "}\n";

	GRCLBase::CompileKernel((const char *)srcStdStr.c_str(),(const char *)fnName.c_str());
}

void clXCorrelate_impl::buildCCMagKernel() {
	// Now we set up our OpenCL kernel
	std::string srcStdStr="";
	std::string fnName = "ComplexToMag";

	srcStdStr += "struct ComplexStruct {\n";
	srcStdStr += "float real;\n";
	srcStdStr += "float imag; };\n";
	srcStdStr += "typedef struct ComplexStruct SComplex;\n";

	srcStdStr += "__kernel void ComplexToMag(__global SComplex * restrict a, __global float * restrict c) {\n";

	srcStdStr += "    size_t index =  get_global_id(0);\n";

	// Stick with float resolution for performance
	if (hasDoubleFMASupport || hasSingleFMASupport) {
		srcStdStr += "	  float tgt_val = fma(a[index].real,a[index].real,a[index].imag*a[index].imag);\n";
	}
	else {
		srcStdStr += "	  float tgt_val = (a[index].real*a[index].real)  + (a[index].imag*a[index].imag);\n";
	}

	srcStdStr += "	  c[index] = sqrt(tgt_val);\n";

	srcStdStr += "}\n";

	try {
		// Create and program from source
		if (ccmag_program) {
			delete ccmag_program;
			ccmag_program = NULL;
		}
		if (ccmag_sources) {
			delete ccmag_sources;
			ccmag_sources = NULL;
		}
		ccmag_sources=new cl::Program::Sources(1, std::make_pair(srcStdStr.c_str(), 0));
		ccmag_program = new cl::Program(*context, *ccmag_sources);

		// Build program
		ccmag_program->build(devices);

		ccmag_kernel=new cl::Kernel(*ccmag_program, (const char *)fnName.c_str());
	}
	catch(cl::Error& e) {
		std::cout << "OpenCL Error compiling kernel for " << fnName << std::endl;
		std::cout << "OpenCL Error " << e.err() << ": " << e.what() << std::endl;
		std::cout << srcStdStr << std::endl;
		exit(0);
	}

	try {
		preferredWorkGroupSizeMultiple = ccmag_kernel->getWorkGroupInfo<CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE>(devices[0]);
	}
	catch(cl::Error& e) {
		std::cout << "Error getting kernel preferred work group size multiple" << std::endl;
		std::cout << "OpenCL Error " << e.err() << ": " << e.what()<< std::endl;
	}

	try {
		maxWorkGroupSize = ccmag_kernel->getWorkGroupInfo<CL_KERNEL_WORK_GROUP_SIZE>(devices[0]);
	}
	catch(cl::Error& e) {
		std::cout << "Error getting kernel preferred work group size multiple" << std::endl;
		std::cout << "OpenCL Error " << e.err() << ": " << e.what()<< std::endl;
	}
}

void clXCorrelate_impl::buildF32SquaredKernel() {
	// Now we set up our OpenCL kernel
	std::string srcStdStr="";
	std::string fnName = "F32Squared";

	srcStdStr += "__kernel void F32Squared(__global float * restrict a, __global float * restrict c) {\n";

	srcStdStr += "    size_t index =  get_global_id(0);\n";

	srcStdStr += "    c[index] = a[index] * a[index];\n";
	srcStdStr += "}\n";

	try {
		// Create and program from source
		if (f32_squared_program) {
			delete f32_squared_program;
			f32_squared_program = NULL;
		}
		if (f32_squared_sources) {
			delete f32_squared_sources;
			f32_squared_sources = NULL;
		}
		f32_squared_sources=new cl::Program::Sources(1, std::make_pair(srcStdStr.c_str(), 0));
		f32_squared_program = new cl::Program(*context, *f32_squared_sources);

		// Build program
		f32_squared_program->build(devices);

		f32_squared_kernel=new cl::Kernel(*f32_squared_program, (const char *)fnName.c_str());
	}
	catch(cl::Error& e) {
		std::cout << "OpenCL Error compiling kernel for " << fnName << std::endl;
		std::cout << "OpenCL Error " << e.err() << ": " << e.what() << std::endl;
		std::cout << srcStdStr << std::endl;
		exit(0);
	}
}

void clXCorrelate_impl::buildFindMaxKernel() {
	// Now we set up our OpenCL kernel
	std::string srcStdStr="";
	std::string fnName = "find_max";

	srcStdStr += "__kernel void find_max( __global float * restrict input, __global float * restrict partial_max, \n";
	srcStdStr += "                         __global int * restrict partial_lag_index, __local float *local_max, __local int *local_lag) {\n";
	srcStdStr += "	int g_id = get_global_id(0);\n";
	srcStdStr += "	uint local_id = get_local_id(0);\n";
	srcStdStr += "  uint group_id = get_group_id(0);\n";
	srcStdStr += "  uint group_size = get_local_size(0);\n";
	// Copy from global to local memory
	srcStdStr += "  local_max[local_id] = input[g_id];\n";
	srcStdStr += "  local_lag[local_id] = g_id;\n";
	srcStdStr += "	barrier(CLK_LOCAL_MEM_FENCE);\n";
	// Loop for computing local max : divide WorkGroup into 2 parts
	srcStdStr += "	for (int stride = group_size/2; stride>0; stride /=2) {\n";
	// Waiting for each check within workgroup
	// Compare elements 2 by 2 between local_id and local_id + stride.
	// The first if ensures the compare only happens for the first half of the stride.
	srcStdStr += "		if (local_id < stride) {\n";
	srcStdStr += "		   if (local_max[local_id + stride]>local_max[local_id]) {\n";
	srcStdStr += "				local_max[local_id] = local_max[local_id + stride];\n";
	srcStdStr += "				local_lag[local_id] = local_lag[local_id + stride];\n";
	srcStdStr += "		   }\n";
	srcStdStr += "		}\n";
	srcStdStr += "		barrier(CLK_LOCAL_MEM_FENCE);\n";
	srcStdStr += "	}\n";
	// Write result into partial_max[group_id]
	srcStdStr += "	if (local_id == 0) {\n";
	srcStdStr += "   	partial_max[group_id] = local_max[0];\n";
	srcStdStr += "   	partial_lag_index[group_id] = local_lag[0];\n";
	srcStdStr += "  }\n";
	srcStdStr += "}\n";

	try {
		// Create and program from source
		if (find_max_program) {
			delete find_max_program;
			find_max_program = NULL;
		}
		if (find_max_sources) {
			delete find_max_sources;
			find_max_sources = NULL;
		}
		find_max_sources=new cl::Program::Sources(1, std::make_pair(srcStdStr.c_str(), 0));
		find_max_program = new cl::Program(*context, *find_max_sources);

		// Build program
		find_max_program->build(devices);

		find_max_kernel=new cl::Kernel(*find_max_program, (const char *)fnName.c_str());
	}
	catch(cl::Error& e) {
		std::cout << "OpenCL Error compiling kernel for " << fnName << std::endl;
		std::cout << "OpenCL Error " << e.err() << ": " << e.what() << std::endl;
		std::cout << srcStdStr << std::endl;
		exit(0);
	}
}

void clXCorrelate_impl::setBufferLength() {
	if (complex_ref_buffer)
		delete complex_ref_buffer;

	if (complex_sig_buffer)
		delete complex_sig_buffer;

	if (ref_mag_buffer)
		delete ref_mag_buffer;

	if (mag_buffer)
		delete mag_buffer;

	if (xx_buffer)
		delete xx_buffer;

	if (yy_buffer)
		delete yy_buffer;

	if (max_result_buffer)
		delete max_result_buffer;

	if (max_lag_buffer)
		delete max_result_buffer;

	if (correlation_factors)
		delete correlation_factors;

	if (d_data_type == DTYPE_COMPLEX) {
		complex_ref_buffer = new cl::Buffer(
				*context,
				CL_MEM_READ_ONLY,
				d_signal_length * d_data_size);

		complex_sig_buffer = new cl::Buffer(
				*context,
				CL_MEM_READ_ONLY,
				d_signal_length * d_data_size);

	}
	else {
		complex_ref_buffer = NULL;
		complex_sig_buffer = NULL;
	}

	ref_mag_buffer = new cl::Buffer(
			*context,
			CL_MEM_READ_WRITE,
			d_signal_length * sizeof(float));

	mag_buffer = new cl::Buffer(
			*context,
			CL_MEM_READ_WRITE,
			d_signal_length * sizeof(float));

	xx_buffer = new cl::Buffer(
			*context,
			CL_MEM_READ_WRITE,
			d_signal_length * sizeof(float));

	yy_buffer = new cl::Buffer(
			*context,
			CL_MEM_READ_WRITE,
			d_signal_length * sizeof(float));

	correlation_factors = new cl::Buffer(
			*context,
			CL_MEM_READ_WRITE,
			max_shift_2 * sizeof(float));

	max_result_buffer = new cl::Buffer(
			*context,
			CL_MEM_WRITE_ONLY,
			num_max_calc_workgroups * sizeof(float));

	max_lag_buffer = new cl::Buffer(
			*context,
			CL_MEM_WRITE_ONLY,
			num_max_calc_workgroups * sizeof(int));

	if (local_max_buffer)
		delete[] local_max_buffer;

	if (local_lag_buffer)
		delete[] local_lag_buffer;

	local_max_buffer_size = num_max_calc_workgroups * sizeof(float);
	local_max_lag_size = num_max_calc_workgroups * sizeof(int);
	workgroup_max_buffer_size = max_calc_workgroup_size*sizeof(float);
	workgroup_max_lag_size = max_calc_workgroup_size*sizeof(int);

	local_max_buffer = new float[num_max_calc_workgroups];
	local_lag_buffer = new int[num_max_calc_workgroups];

	returned_correlation_factors = new float[d_num_inputs-1];
	returned_corrective_lag = new int[d_num_inputs-1];

	curBufferSize=d_signal_length;
}

/*
 * Our virtual destructor.
 */
clXCorrelate_impl::~clXCorrelate_impl()
{
	bool retval = stop();
}

bool clXCorrelate_impl::stop() {
	curBufferSize = 0;

	if (proc_thread) {
		stop_thread = true;

		while (threadRunning)
			usleep(10);

		delete proc_thread;
		proc_thread = NULL;

		if (d_input_buffer_complex) {
			delete d_input_buffer_complex;
			d_input_buffer_complex = NULL;
		}

		if (d_input_buffer_real) {
			delete d_input_buffer_real;
			d_input_buffer_real = NULL;
		}
	}

	try {
		if (queue2 != NULL) {
			delete queue2;
			queue2 = NULL;
		}
	}
	catch(...) {
		queue2=NULL;
		std::cout<<"queue2 delete error." << std::endl;
	}


	// Additional Kernels
	// Complex to mag
	try {
		if (ccmag_kernel != NULL) {
			delete ccmag_kernel;
			ccmag_kernel=NULL;
		}
	}
	catch (...) {
		ccmag_kernel=NULL;
		std::cout<<"ccmag kernel delete error." << std::endl;
	}

	try {
		if (ccmag_program != NULL) {
			delete ccmag_program;
			ccmag_program = NULL;
		}
	}
	catch(...) {
		ccmag_program = NULL;
		std::cout<<"ccmag program delete error." << std::endl;
	}

	// F32 squared
	try {
		if (f32_squared_kernel != NULL) {
			delete f32_squared_kernel;
			f32_squared_kernel=NULL;
		}
	}
	catch (...) {
		f32_squared_kernel=NULL;
		std::cout<<"f32 squared kernel delete error." << std::endl;
	}

	try {
		if (f32_squared_program != NULL) {
			delete f32_squared_program;
			f32_squared_program = NULL;
		}
	}
	catch(...) {
		f32_squared_program = NULL;
		std::cout<<"f32 squared program delete error." << std::endl;
	}

	// FindMax
	try {
		if (find_max_kernel != NULL) {
			delete find_max_kernel;
			find_max_kernel=NULL;
		}
	}
	catch (...) {
		find_max_kernel=NULL;
		std::cout<<"find_max kernel delete error." << std::endl;
	}

	try {
		if (find_max_program != NULL) {
			delete find_max_program;
			find_max_program = NULL;
		}
	}
	catch(...) {
		find_max_program = NULL;
		std::cout<<"find_max program delete error." << std::endl;
	}

	// Buffers
	if (complex_ref_buffer) {
		delete complex_ref_buffer;
		complex_ref_buffer = NULL;
	}

	if (complex_sig_buffer) {
		delete complex_sig_buffer;
		complex_sig_buffer = NULL;
	}

	if (ref_mag_buffer) {
		delete ref_mag_buffer;
		ref_mag_buffer = NULL;
	}

	if (mag_buffer) {
		delete mag_buffer;
		mag_buffer = NULL;
	}

	if (xx_buffer) {
		delete xx_buffer;
		xx_buffer = NULL;
	}

	if (yy_buffer) {
		delete yy_buffer;
		yy_buffer = NULL;
	}

	if (correlation_factors) {
		delete correlation_factors;
		correlation_factors = NULL;
	}

	if (local_max_buffer) {
		delete[] local_max_buffer;
		local_max_buffer = NULL;
	}

	if (local_lag_buffer) {
		delete[] local_lag_buffer;
		local_lag_buffer = NULL;
	}

	if (returned_correlation_factors) {
		delete[] returned_correlation_factors;
		returned_correlation_factors = NULL;
	}

	if (returned_corrective_lag) {
		delete[] returned_corrective_lag;
		returned_corrective_lag = NULL;
	}

	return GRCLBase::stop();
}

void clXCorrelate_impl::complex_to_mag(cl::CommandQueue *thisqueue, cl::Buffer *input_buffer, cl::Buffer *output_buffer, const void *input_items) {

	queue->enqueueWriteBuffer(*input_buffer,CL_FALSE,0,signal_byte_size,input_items);

	// Set kernel args
	ccmag_kernel->setArg(0, *input_buffer);
	ccmag_kernel->setArg(1, *output_buffer);

	thisqueue->enqueueNDRangeKernel(
			*ccmag_kernel,
			cl::NullRange,
			cl::NDRange(d_signal_length),
			localWGSize);
}

void clXCorrelate_impl::f32_squared(cl::Buffer *input_buffer, cl::Buffer *output_buffer) {
	// Set kernel args
	f32_squared_kernel->setArg(0, *input_buffer);
	f32_squared_kernel->setArg(1, *output_buffer);

	queue->enqueueNDRangeKernel(
			*f32_squared_kernel,
			cl::NullRange,
			cl::NDRange(d_signal_length),
			localWGSize);
}

void clXCorrelate_impl::find_max(cl::Buffer *input_buffer, float& corr, int& lag) {

#define USE_KERNEL

#ifdef USE_KERNEL
	// Set kernel args
	find_max_kernel->setArg(0, *input_buffer);
	find_max_kernel->setArg(1, *max_result_buffer);
	find_max_kernel->setArg(2, *max_lag_buffer);
	// Local workgroup memory blocks.  Should be workgroupsize*sizeof(type).
	find_max_kernel->setArg(3, workgroup_max_buffer_size, NULL);
	find_max_kernel->setArg(4, workgroup_max_lag_size, NULL);

	queue->enqueueNDRangeKernel(
			*find_max_kernel,
			cl::NullRange,
			cl::NDRange(max_shift_2),
			maxCalcWGSize);

	// Now we finally need to read the buffers.
	// Using 2 queues to get some parallelism in the copies.
	queue->enqueueReadBuffer(*max_result_buffer,CL_FALSE,0,local_max_buffer_size,(void *)local_max_buffer);
	queue2->enqueueReadBuffer(*max_lag_buffer,CL_FALSE,0,local_max_lag_size,(void *)local_lag_buffer);
	queue->finish(); // Queued the memory reads with the first one not waiting, so have to wait for them to finish.
	queue2->finish();

	if (num_max_calc_workgroups > 1) {
		// we still have some more processing to do.
		int cur_max_index = 0;
		float cur_max = local_max_buffer[0];

		for (int i=1;i<num_max_calc_workgroups;i++) {
			if (local_max_buffer[i] > cur_max) {
				cur_max_index = i;
				cur_max = local_max_buffer[i];
			}
		}

		corr = cur_max;
		lag = local_lag_buffer[cur_max_index] - max_shift;
	}
	else {
		corr = local_max_buffer[0];
		lag = local_lag_buffer[0] - max_shift;
	}
#else
	// This is just for testing purposes to validate the other kernels were calculating correctly.

	// let's get the matrix back and go through it by hand.
	float *tmp_buffer = new float[max_shift_2];

	queue->enqueueReadBuffer(*input_buffer,CL_FALSE,0,max_shift_2*sizeof(float),(void *)tmp_buffer);
	queue->finish();

	float cur_max = tmp_buffer[0];
	int cur_index  = 0;

	for (int i=1;i<max_shift_2;i++) {
		if (tmp_buffer[i] > cur_max) {
			cur_max = tmp_buffer[i];
			cur_index = i;
		}
	}

	corr = cur_max;
	lag = cur_index - max_shift;

	delete[] tmp_buffer;
#endif
}

void clXCorrelate_impl::correlate() {
	// Execute the correlation kernel
	kernel->setArg(0, *ref_mag_buffer);
	kernel->setArg(1, *mag_buffer);
	kernel->setArg(2, *xx_buffer);
	kernel->setArg(3, *yy_buffer);
	kernel->setArg(4, *correlation_factors);

	queue->enqueueNDRangeKernel(
			*kernel,
			cl::NullRange,
			cl::NDRange(max_shift_2),
			localWGSize);
}

int
clXCorrelate_impl::work_test(int noutput_items,
		gr_vector_const_void_star &input_items,
		gr_vector_void_star &output_items)
{
	if (noutput_items < d_signal_length) {
		return 0;
	}

	gr::thread::scoped_lock guard(d_mutex);

	if (d_decim_frames > 1) {
		// Let's see if we should drop some frames.
		if ((cur_frame_counter++ % d_decim_frames) == 0) {
			cur_frame_counter = 1;
		}
		else {
			return d_signal_length;
		}
	}

	// Processing note:
	// We set output multiple to d_signal_length in the constructor,
	// so noutput_items will be a multiple of, and >= d_signal_length.

	// Calculate the reference signal mag just once
	if (d_data_type == DTYPE_COMPLEX) {
		complex_to_mag(queue2,complex_ref_buffer, ref_mag_buffer,input_items[0]);
	}
	else {
		// already got complex->mag or float inputs, just need to move it
		// to the mag buffer
		queue2->enqueueWriteBuffer(*ref_mag_buffer,CL_FALSE,0,signal_byte_size,input_items[0]);
	}

	// Calculate the reference signal squared just once.
	f32_squared(ref_mag_buffer, xx_buffer);

	// Cross correlation will shift signal noutput_items forward and backward
	// and calculate a normalized correlation factor for each shift.

	float corr;
	int lag;

	for (int signal_index=1;signal_index<d_num_inputs;signal_index++) {
		if (d_data_type == DTYPE_COMPLEX) {
			complex_to_mag(queue, complex_sig_buffer, mag_buffer,input_items[signal_index]);
		}
		else {
			queue->enqueueWriteBuffer(*mag_buffer,CL_FALSE,0,signal_byte_size,input_items[signal_index]);
		}

		// Calc mag squared just once per comparitive signal.
		f32_squared(mag_buffer, yy_buffer);

		if (signal_index == 1) {
			queue2->finish();
		}

		// Perform the correlation
		correlate();

		// Perform the search for best correlation
		find_max(correlation_factors, returned_correlation_factors[signal_index-1], returned_corrective_lag[signal_index-1]);
	}

	// Tell runtime system how many output items we produced.
	return d_signal_length;
}

int
clXCorrelate_impl::work(int noutput_items,
		gr_vector_const_void_star &input_items,
		gr_vector_void_star &output_items)
{
	if (noutput_items < d_signal_length) {
		return 0;
	}

	gr::thread::scoped_lock guard(d_mutex);

	if (!d_async) {
		if (d_decim_frames > 1) {
			// Let's see if we should drop some frames.
			if ((cur_frame_counter++ % d_decim_frames) == 0) {
				cur_frame_counter = 1;
			}
			else {
				return d_signal_length;
			}
		}

		// Processing note:
		// We set output multiple to d_signal_length in the constructor,
		// so noutput_items will be a multiple of, and >= d_signal_length.

		// Calculate the reference signal mag just once
		if (d_data_type == DTYPE_COMPLEX) {
			complex_to_mag(queue2,complex_ref_buffer, ref_mag_buffer,input_items[0]);
		}
		else {
			// already got complex->mag or float inputs, just need to move it
			// to the mag buffer
			queue2->enqueueWriteBuffer(*ref_mag_buffer,CL_FALSE,0,signal_byte_size,input_items[0]);
		}

		// Calculate the reference signal squared just once.
		f32_squared(ref_mag_buffer, xx_buffer);

		// Cross correlation will shift signal noutput_items forward and backward
		// and calculate a normalized correlation factor for each shift.

		float corr;
		int lag;

		for (int signal_index=1;signal_index<d_num_inputs;signal_index++) {
			if (d_data_type == DTYPE_COMPLEX) {
				complex_to_mag(queue, complex_sig_buffer, mag_buffer,input_items[signal_index]);
			}
			else {
				queue->enqueueWriteBuffer(*mag_buffer,CL_FALSE,0,signal_byte_size,input_items[signal_index]);
			}

			// Calc mag squared just once per comparitive signal.
			f32_squared(mag_buffer, yy_buffer);

			if (signal_index == 1) {
				queue2->finish();
			}

			// Perform the correlation
			correlate();

			// Perform the search for best correlation
			find_max(correlation_factors, returned_correlation_factors[signal_index-1], returned_corrective_lag[signal_index-1]);
		}

		pmt::pmt_t meta = pmt::make_dict();
		pmt::pmt_t corr_out(pmt::init_f32vector(d_num_inputs-1,returned_correlation_factors));
		meta = pmt::dict_add(meta, pmt::mp("corrvect"), corr_out);
		pmt::pmt_t lag_out(pmt::init_s32vector(d_num_inputs-1,returned_corrective_lag));
		meta = pmt::dict_add(meta, pmt::mp("corrective_lags"), lag_out);

		pmt::pmt_t pdu = pmt::cons(meta, pmt::PMT_NIL);
		message_port_pub(pmt::mp("corr"), pdu);
	}
	else {
		// Async mode.  See if the thread is currently processing any data or not.
		if (!thread_process_data) {
			if (d_decim_frames > 1) {
				// For async mode, we'll need to do this here.
				// Let's see if we should drop some frames.
				if ((cur_frame_counter++ % d_decim_frames) == 0) {
					cur_frame_counter = 1;
				}
				else {
					return d_signal_length;
				}
			}

			// Copy the input signals to our local buffer for processing and trigger.
			for (int i=0;i<d_num_inputs;i++) {
				if (d_data_type == DTYPE_COMPLEX) {
					memcpy(&d_input_buffer_complex[d_signal_length*i],input_items[i],d_signal_length*d_data_size);
				}
				else {
					memcpy(&d_input_buffer_real[d_signal_length*i],input_items[i],d_signal_length*d_data_size);
				}
			}

			// thread_is_processing will only be FALSE if thread_process_data == false on the first pass,
			// in which case we don't want to send any pmt's.  Otherwise, we're in async pickup mode.
			if (thread_is_processing) {
				pmt::pmt_t meta = pmt::make_dict();
				pmt::pmt_t corr_out(pmt::init_f32vector(d_num_inputs-1,returned_correlation_factors));
				meta = pmt::dict_add(meta, pmt::mp("corrvect"), corr_out);
				pmt::pmt_t lag_out(pmt::init_s32vector(d_num_inputs-1,returned_corrective_lag));
				meta = pmt::dict_add(meta, pmt::mp("corrective_lags"), lag_out);

				pmt::pmt_t pdu = pmt::cons(meta, pmt::PMT_NIL);
				message_port_pub(pmt::mp("corr"), pdu);
			}

			// clear the flag
			thread_process_data = true;
		}
		// The else with this is that the thread is processing, and we're just going to pass through.
	}

	// Tell runtime system how many output items we produced.
	return d_signal_length;
}

void clXCorrelate_impl::runThread() {
	threadRunning = true;

	while (!stop_thread) {
		if (thread_process_data) {
			// This is really a one-time variable set to true on the first pass.
			thread_is_processing = true;

			// Trigger received to process data.
			// Calculate the reference signal mag just once
			if (d_data_type == DTYPE_COMPLEX) {
				complex_to_mag(queue2,complex_ref_buffer, ref_mag_buffer,d_input_buffer_complex);
			}
			else {
				// already got complex->mag or float inputs, just need to move it
				// to the mag buffer
				queue2->enqueueWriteBuffer(*ref_mag_buffer,CL_FALSE,0,signal_byte_size,d_input_buffer_real);
			}

			// Calculate the reference signal squared just once.
			f32_squared(ref_mag_buffer, xx_buffer);

			// Cross correlation will shift signal noutput_items forward and backward
			// and calculate a normalized correlation factor for each shift.

			float corr;
			int lag;

			for (int signal_index=1;signal_index<d_num_inputs;signal_index++) {
				if (d_data_type == DTYPE_COMPLEX) {
					complex_to_mag(queue, complex_sig_buffer, mag_buffer,&d_input_buffer_complex[d_signal_length * signal_index]);
				}
				else {
					queue->enqueueWriteBuffer(*mag_buffer,CL_FALSE,0,signal_byte_size,&d_input_buffer_real[d_signal_length * signal_index]);
				}

				// Calc mag squared just once per comparitive signal.
				f32_squared(mag_buffer, yy_buffer);

				if (signal_index == 1) {
					queue2->finish();
				}

				// Perform the correlation
				correlate();

				// Perform the search for best correlation
				find_max(correlation_factors, returned_correlation_factors[signal_index-1], returned_corrective_lag[signal_index-1]);
			}

			// clear the trigger.  This will inform that data is ready.
			thread_process_data = false;
		}
		usleep(10);
	}

	threadRunning = false;
}

} /* namespace clenabled */
} /* namespace gr */

