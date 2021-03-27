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
#include "clXEngine_impl.h"
#include <volk/volk.h>

// Some carry-forward tricks from file_sink_base.cc
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

// win32 (mingw/msvc) specific
#ifdef HAVE_IO_H
#include <io.h>
#endif
#ifdef O_BINARY
#define	OUR_O_BINARY O_BINARY
#else
#define	OUR_O_BINARY 0
#endif

// should be handled via configure
#ifdef O_LARGEFILE
#define	OUR_O_LARGEFILE	O_LARGEFILE
#else
#define	OUR_O_LARGEFILE 0
#endif

namespace gr {
namespace clenabled {

clXEngine::sptr
clXEngine::make(int openCLPlatformType,int devSelector,int platformId, int devId, bool setDebug, int data_type, int polarization, int num_inputs,
		  int output_format, int first_channel, int num_channels, int integration, std::vector<std::string> antenna_list,
		  bool output_file, std::string file_base, int rollover_size_mb, bool internal_synchronizer,
		  long sync_timestamp, std::string object_name, double starting_chan_center_freq, double channel_width, bool disable_output)
{
	int data_size = 1;

	switch (data_type) {
	case DTYPE_COMPLEX:
		data_size = sizeof(gr_complex);
		break;
	case DTYPE_BYTE:
		data_size = sizeof(char)*2;  // Need 2 bytes to make up the complex input (will still be complex, just char, not float)
		break;
	case DTYPE_PACKEDXY:
		data_size = sizeof(char);  // Need 2 bytes to make up the packed xy input
		break;
	}

	return gnuradio::get_initial_sptr
			(new clXEngine_impl(openCLPlatformType, devSelector, platformId, devId, setDebug, data_type, data_size,polarization, num_inputs,
					output_format, first_channel, num_channels, integration, antenna_list, output_file, file_base, rollover_size_mb,
					internal_synchronizer,sync_timestamp, object_name, starting_chan_center_freq, channel_width, disable_output));
}

/*
 * The private constructor
 */
clXEngine_impl::clXEngine_impl(int openCLPlatformType,int devSelector,int platformId, int devId, bool setDebug, int data_type, int data_size, int polarization, int num_inputs,
		  int output_format, int first_channel, int num_channels, int integration, std::vector<std::string> antenna_list,
			  bool output_file, std::string file_base, int rollover_size_mb, bool internal_synchronizer,
			  long sync_timestamp, std::string object_name, double starting_chan_center_freq, double channel_width, bool disable_output)
: gr::block("clXEngine",
		gr::io_signature::make(2, num_inputs*(data_type==DTYPE_PACKEDXY?1:polarization), num_channels*(data_type==DTYPE_PACKEDXY?2:data_size)),
		gr::io_signature::make(0, 0, 0)),
		GRCLBase(data_type, data_size,openCLPlatformType,devSelector,platformId,devId,setDebug),
		d_npol(polarization), d_num_inputs(num_inputs), d_output_format(output_format),d_first_channel(first_channel),
		d_num_channels(num_channels), d_integration_time(integration), integration_tracker(0),d_data_type(data_type), d_data_size(data_size),
		d_output_file(output_file), d_file_base(file_base), d_rollover_size_mb(rollover_size_mb),
		d_use_internal_synchronizer(internal_synchronizer),
		d_antenna_list(antenna_list),
		d_sync_timestamp(sync_timestamp),
		d_object_name(object_name),
		d_starting_chan_center_freq(starting_chan_center_freq),
		d_channel_width(channel_width),
		d_disable_output(disable_output)

{
	if (num_inputs < 2) {
		GR_LOG_ERROR(d_logger, "Please specify at least 2 inputs to correlate.");
		throw std::out_of_range ("Please specify at least 2 inputs to correlate.");
	}

	if (internal_synchronizer) {
		if ((integration % 16) > 0) {
			GR_LOG_ERROR(d_logger, "For the ATA synchronizer, the number of integration frames should be a multiple of 16 to align with blocks coming from the SNAP.");
			throw std::out_of_range ("ATA xengine: The number of integration frames should be a multiple of 16 to align with blocks coming from the SNAP.");
		}
	}

	if (d_disable_output)
		d_output_file = false;

	if (d_output_file) {
		if ((d_rollover_size_mb) > 0) {
			d_rollover_files = true;
			d_bytesWritten = 0;
			rollover_size_bytes = d_rollover_size_mb * 1000000;
		}
		else {
			d_rollover_files = false;
		}

		bool retval = open();

		if (!retval) {
			std::string errmsg = "[X-Engine] can't open file: ";
			errmsg += filename;

			GR_LOG_ERROR(d_logger,errmsg);

			throw std::runtime_error (errmsg);
		}
	}

	// GR's YML doesn't allow a list of strings as a _vector type.  So we're
	// Working the system a bit.  If no antennas are defined, the list could be [''].
	// So since we're supposed to have more than 1 antenna anyway we can use that here
	// to check.
	if (d_antenna_list.size() > 1) {
		int num_ants = d_antenna_list.size();

		str_antenna_list = "[";

		int i=0;

		for (auto ant=d_antenna_list.begin(); ant!=d_antenna_list.end(); ++ant) {
		    str_antenna_list += "\"" + *ant + "\"";

		    if (i < (num_ants-1)) {
			    str_antenna_list += ",";
		    }

		    i++;
		}


		str_antenna_list += "]";
	}
	else {
		str_antenna_list = "[]";
	}

	d_synchronized = false;
	tag_list = new uint64_t[d_num_inputs];

	// Override just in case pol doesn't come through right.

	if (d_data_type == DTYPE_PACKEDXY) {
		d_npol = 2;
	}

	// See "Accelerating Radio Astronomy Cross-Correlation with Graphics Processing Units" by M. A. Clark
	// and xGPU on github for reference documentation and reference implementation.

	d_num_baselines = (d_num_inputs+1)*d_num_inputs / 2;

	// Input size is the size of one sampling vector.  Basically the channel's data
	input_size = d_num_channels * d_data_size;

	num_chan_x2 = d_num_channels * 2;
	frame_size = d_num_channels * d_num_inputs * d_npol;
	frame_size_times_integration = frame_size * d_integration_time;
	frame_size_times_integration_bytes = frame_size_times_integration * d_data_size;

	channels_times_baselines = d_num_channels*d_num_baselines;

	current_write_buffer = 1;

	if (d_output_format == CLXCORR_TRIANGULAR_ORDER) {
		// This is only the lower triangular matrix size (including the autocorrelation diagonal
		matrix_flat_length = d_num_channels * d_num_baselines * d_npol * d_npol;
	}
	else {
		// This is the full matrix
		matrix_flat_length = d_num_channels * (d_num_inputs*d_num_inputs*d_npol*d_npol);
	}

	buildKernel();
	buildCharToComplexKernel();

	int input_matrix_type;

	unsigned long gpu_memory_allocated = 0;

	// Want to precalc in case these throw an error:
	if (d_data_type != DTYPE_COMPLEX) {
		// char input matrix
		gpu_memory_allocated += frame_size_times_integration * d_data_size;
	}
	// input_matrix_buffer
	gpu_memory_allocated += frame_size_times_integration * sizeof(gr_complex);
	// output correlation buffer
	gpu_memory_allocated += matrix_flat_length * sizeof(gr_complex);

	std::stringstream msg_stream;
	msg_stream << "X-Engine Startup Parameters:" << std::endl;
	msg_stream << "Total GPU memory requested: " << gpu_memory_allocated / 1e6 << " MB (" << gpu_memory_allocated << " bytes)" << std::endl;

	if (d_data_type == DTYPE_COMPLEX) {
		msg_stream << "GPU Input Buffer Size (bytes): " << frame_size_times_integration * sizeof(gr_complex) << std::endl;
	}
	else {
		msg_stream << "GPU Input Buffer Size (bytes): " << frame_size_times_integration * d_data_size << std::endl;
	}

	msg_stream << "Integrated output frame size (bytes): " << matrix_flat_length*sizeof(gr_complex) << std::endl <<
				  "Number of Antennas: " << num_inputs << std::endl <<
				  "Starting Channel: " << d_first_channel << std::endl <<
				  "Number of Channels: " << d_num_channels << std::endl <<
				  "Number of Polarizations: " << d_npol << std::endl <<
				  "Number of Baselines (including autocorrelations): " << d_num_baselines << std::endl <<
				  "Number of Integration Frames: " << d_integration_time;

	GR_LOG_INFO(d_logger,msg_stream.str());

	if (d_data_type == DTYPE_COMPLEX) {
		input_matrix_type = CL_MEM_READ_ONLY;
		char_matrix_buffer = NULL; // Don't need it in this case
	}
	else {
		char_matrix_buffer = new cl::Buffer(
				*context,
				CL_MEM_READ_ONLY,
				frame_size_times_integration * d_data_size);

		input_matrix_type = CL_MEM_READ_WRITE;
	}

	// This will always be complex, regardless of the block input type.
	// For byte, we use another kernel to convert into this buffer prior to
	// running the cross correlation.

	input_matrix_buffer = new cl::Buffer(
			*context,
			input_matrix_type,
			frame_size_times_integration * sizeof(gr_complex));

	// Output will always be complex
	output_size = matrix_flat_length * sizeof(gr_complex);

	cross_correlation_buffer = new cl::Buffer(
			*context,
			CL_MEM_READ_WRITE,
			output_size);

	message_port_register_out(pmt::mp("xcorr"));
	message_port_register_out(pmt::mp("sync"));

	if (d_use_internal_synchronizer) {
		set_tag_propagation_policy(TPP_DONT);
		// The SNAP outputs packets in 16-time step blocks.  So let's take advantage of that here.
		set_output_multiple(16);
	}

	num_procs = 1;

}

bool clXEngine_impl::start() {
	// 2 buffers will allow us to process one in a worker thread while the other
	// is being loaded.
	size_t mem_alignment = volk_get_alignment();

	if (d_data_type == DTYPE_COMPLEX) {
		// complex_input1 = new gr_complex[frame_size_times_integration];
		// complex_input2 = new gr_complex[frame_size_times_integration];
		complex_input1 = (gr_complex *)volk_malloc(frame_size_times_integration*sizeof(gr_complex), mem_alignment);
		complex_input2 = (gr_complex *)volk_malloc(frame_size_times_integration*sizeof(gr_complex), mem_alignment);
		complex_input = complex_input1;
		thread_complex_input = complex_input;
	}
	else {
		// char_input1 = new char[frame_size_times_integration_bytes];
		// char_input2 = new char[frame_size_times_integration_bytes];
		char_input1 = (char *)volk_malloc(frame_size_times_integration_bytes, mem_alignment);
		char_input2 = (char *)volk_malloc(frame_size_times_integration_bytes, mem_alignment);
		char_input = char_input1;
		thread_char_input = char_input;
	}
	output_matrix1 = new gr_complex[matrix_flat_length];
	output_matrix2 = new gr_complex[matrix_flat_length];
	output_matrix = output_matrix1;
	thread_output_matrix = output_matrix;

	proc_thread = new boost::thread(boost::bind(&clXEngine_impl::runThread, this));
	return true;
}

void
clXEngine_impl::forecast (int noutput_items, gr_vector_int &ninput_items_required)
{
	for (int i=0;i< ninput_items_required.size();i++) {
		ninput_items_required[i] = noutput_items;
	}
}


bool clXEngine_impl::open()
{
	gr::thread::scoped_lock guard(d_fpmutex);	// hold mutex for duration of this function
	// we use the open system call to get access to the O_LARGEFILE flag.
	d_wrote_json = false;

	int fd;
	int flags;
	flags = O_WRONLY|O_CREAT|O_TRUNC|OUR_O_LARGEFILE|OUR_O_BINARY;

	filename = d_file_base;

	if (d_rollover_files) {
		std::string newStr = std::to_string(current_rollover_index++);

		while (newStr.length() < 3)
			newStr = "0" + newStr;

		filename += "_" + newStr;
	}

	if((fd = ::open(filename.c_str(), flags, 0664)) < 0){
		GR_LOG_ERROR(d_logger,"Error in initial 0664 open");
		return false;
	}

	if(d_fp) {		// if we've already got a new one open, close it
		fclose(d_fp);
		d_fp = NULL;
	}

	if((d_fp = fdopen (fd, "wb")) == NULL) {
		::close(fd);        // don't leak file descriptor if fdopen fails.
		GR_LOG_ERROR(d_logger,"open-for-write returned NULL");
		return false;
	}

	// Finalize setup
	bool fileOpen = d_fp != 0;

	d_bytesWritten = 0;

	return fileOpen;
}

void clXEngine_impl::write_json(long seq_num) {
	   FILE * pFile;

	   std::string json_file = filename;
	   json_file += ".json";

	   try {
		   pFile = fopen (json_file.c_str(),"w");
	   }
	   catch(...) {
		   GR_LOG_ERROR(d_logger,"Error writing data json descriptor file " + json_file);
		   return;
	   }

	   fprintf(pFile,"{\n\"sync_timestamp\":%ld,\n\"first_seq_num\":%ld,\n\"object_name\":\"%s\",\n\"num_baselines\":%d,\n\"first_channel\":%d,\n\"first_channel_center_freq\":%f,\n\"channels\":%d,\n\"channel_width\":%f,\n\"polarizations\":%d,\n\"antennas\":%d,\n\"antenna_names\":%s,\n\"ntime\":%d,\n\"samples_per_block\":%ld,\n\"bytes_per_block\":%ld,\n\"data_type\":\"cf32_le\",\n\"data_format\": \"triangular order\"\n}\n",
			   d_sync_timestamp, seq_num, d_object_name.c_str(), d_num_baselines, d_first_channel, d_starting_chan_center_freq, d_num_channels, d_channel_width, d_npol, d_num_inputs, str_antenna_list.c_str(), d_integration_time, matrix_flat_length, matrix_flat_length*sizeof(gr_complex));
	   fclose (pFile);

	   d_wrote_json = true;
}

void clXEngine_impl::close() {
	gr::thread::scoped_lock guard(d_fpmutex);	// hold mutex for duration of this function
	if (d_fp) {
		fclose(d_fp);
		d_fp = NULL;
	}
}

bool clXEngine_impl::stop() {
	if (proc_thread) {
		stop_thread = true;

		while (threadRunning)
			usleep(10);

		delete proc_thread;
		proc_thread = NULL;
	}

	close();

	if (complex_input1) {
		// delete[] complex_input1;
		volk_free(complex_input1);
		complex_input1 = NULL;
	}

	if (complex_input2) {
		// delete[] complex_input2;
		volk_free(complex_input2);
		complex_input2 = NULL;
	}

	if (char_input1) {
		// delete[] char_input1;
		volk_free(char_input1);
		char_input1 = NULL;
	}

	if (char_input2) {
		// delete[] char_input2;
		volk_free(char_input2);
		char_input2 = NULL;
	}

	if (output_matrix1) {
		delete[] output_matrix1;
		output_matrix1 = NULL;
	}

	if (output_matrix2) {
		delete[] output_matrix2;
		output_matrix2 = NULL;
	}

	if (char_matrix_buffer) {
		delete char_matrix_buffer;
		char_matrix_buffer = NULL;
	}

	if (input_matrix_buffer) {
		delete input_matrix_buffer;
		input_matrix_buffer = NULL;
	}

	if (cross_correlation_buffer) {
		delete cross_correlation_buffer;
		cross_correlation_buffer = NULL;
	}

	// Additional Kernels
	// Char to Complex
	try {
		if (char_to_cc_kernel != NULL) {
			delete char_to_cc_kernel;
			char_to_cc_kernel=NULL;
		}
	}
	catch (...) {
		char_to_cc_kernel=NULL;
		std::cout<<"ccmag kernel delete error." << std::endl;
	}

	try {
		if (char_to_cc_program != NULL) {
			delete char_to_cc_program;
			char_to_cc_program = NULL;
		}
	}
	catch(...) {
		char_to_cc_program = NULL;
		std::cout<<"ccmag program delete error." << std::endl;
	}

	if (tag_list) {
		delete[] tag_list;
		tag_list = NULL;
	}

	return true;
}

/*
 * Our virtual destructor.
 */
clXEngine_impl::~clXEngine_impl()
{
	bool ret_val = stop();
}

void clXEngine_impl::buildKernel() {
	std::string srcStdStr="";
	std::string fnName = "XCorrelate";

	// Use #defines to not have to pass them in as params since they won't change
	// This will save time on unchanging param transfers at runtime.
	srcStdStr += "#define d_num_channels " + std::to_string(d_num_channels) + "\n";
	srcStdStr += "#define d_num_baselines " + std::to_string(d_num_baselines) + "\n";
	srcStdStr += "#define d_integration_time " + std::to_string(d_integration_time) + "\n";
	srcStdStr += "#define d_num_inputs " + std::to_string(d_num_inputs) + "\n";
	srcStdStr += "#define d_npol " + std::to_string(d_npol) + "\n";
	// frame_size = inputs * channels * pol
	srcStdStr += "#define d_frame_size " + std::to_string(frame_size) + "\n";
	srcStdStr += "\n"; // Just for legibility

	srcStdStr += "struct ComplexStruct {\n";
	srcStdStr += "float real;\n";
	srcStdStr += "float imag; };\n";
	srcStdStr += "typedef struct ComplexStruct XComplex;\n";

	srcStdStr += "__attribute__((always_inline)) inline void cxmac(XComplex* accum, XComplex* z0, XComplex* z1) {\n";
	if (hasDoubleFMASupport || hasSingleFMASupport) {
		srcStdStr += "	accum->real += fma(z0->real,z1->real,(z0->imag * z1->imag));\n";
		srcStdStr += "	accum->imag += fma(z0->imag, z1->real, (-z0->real * z1->imag));\n";
	}
	else {
		srcStdStr += "	accum->real += z0->real * z1->real + z0->imag * z1->imag;\n";
		srcStdStr += "	accum->imag += z0->imag * z1->real - z0->real * z1->imag;\n";
	}
	srcStdStr += "}\n\n";

	srcStdStr += "__kernel void XCorrelate(__global XComplex * restrict input_matrix, __global XComplex * restrict cross_correlation) {\n";
	srcStdStr += "size_t i =  get_global_id(0);\n";

	srcStdStr += "int f = i/d_num_baselines;\n";
	srcStdStr += "int k = i - f*d_num_baselines;\n";
	if (hasDoubleFMASupport || hasSingleFMASupport) {
		srcStdStr += "int station1 = -0.5 + sqrt(fma(2.0,k,0.25));\n";
	}
	else {
		srcStdStr += "int station1 = -0.5 + sqrt(0.25 + 2*k);\n";
	}
	srcStdStr += "int station2 = k - ((station1+1)*station1)/2;\n";
	srcStdStr += "XComplex sumXX; sumXX.real = 0.0; sumXX.imag = 0.0;\n";
	srcStdStr += "XComplex sumXY; sumXY.real = 0.0; sumXY.imag = 0.0;\n";
	srcStdStr += "XComplex sumYX; sumYX.real = 0.0; sumYX.imag = 0.0;\n";
	srcStdStr += "XComplex sumYY; sumYY.real = 0.0; sumYY.imag = 0.0;\n";
	srcStdStr += "XComplex inputRowX, inputRowY, inputColX, inputColY;\n";

	srcStdStr += "for(int t=0; t<d_integration_time; t++){\n";
	// Had to adapt index lookups.  xGPU was expecting [t][freq][station][pol],
	// But we have [t][station][freq][pol]
	// So the index1/2 calcs are different than xGPU's
	/*
	srcStdStr += "  int index_base = (t*d_num_channels + f)*d_num_inputs;\n";
	srcStdStr += "  int index1 = (index_base + station1)*d_npol;\n";
	srcStdStr += "  int index2 = (index_base + station2)*d_npol;\n";
	*/
	srcStdStr += "  int index1 = t*d_frame_size + (station1 * d_num_channels + f) * d_npol;\n";
	srcStdStr += "  int index2 = t*d_frame_size + (station2 * d_num_channels + f) * d_npol;\n";

	srcStdStr += "	inputRowX = input_matrix[index1];\n";
	srcStdStr += "	inputColX = input_matrix[index2];\n";

	srcStdStr += "	cxmac(&sumXX, &inputRowX, &inputColX);\n";
	// Doing the If's at compile time saves the kernels from having to synchronize on the if's
	// so they can just run.
	if (d_npol > 1) {
		srcStdStr += "	inputRowY = input_matrix[index1 + 1];\n";
		srcStdStr += "	inputColY = input_matrix[index2 + 1];\n";

		srcStdStr += "	cxmac(&sumXY, &inputRowX, &inputColY);\n";
		srcStdStr += "	cxmac(&sumYX, &inputRowY, &inputColX);\n";
		srcStdStr += "	cxmac(&sumYY, &inputRowY, &inputColY);\n";
	}
	srcStdStr += "}\n"; // End for loop

	if (d_npol == 1) {
		srcStdStr += "cross_correlation[i    ] = sumXX;\n";
	}
	else {
		srcStdStr += "int four_i = 4*i;\n";
		srcStdStr += "cross_correlation[four_i    ] = sumXX;\n";
		srcStdStr += "cross_correlation[four_i + 1] = sumXY;\n";
		srcStdStr += "cross_correlation[four_i + 2] = sumYX;\n";
		srcStdStr += "cross_correlation[four_i + 3] = sumYY;\n";
	}

	srcStdStr += "}\n"; // End function

	GRCLBase::CompileKernel((const char *)srcStdStr.c_str(),(const char *)fnName.c_str());
}

void clXEngine_impl::buildCharToComplexKernel() {
	// Now we set up our OpenCL kernel
	std::string srcStdStr="";
	std::string fnName = "CharToComplex";

	// Switch division to multiplication for speed.

	if (d_data_type == DTYPE_PACKEDXY) {
		// Need a two's complement lookup table.
		srcStdStr += "__constant char twosComplementLUT[16] = {0, 1, 2, 3, 4, 5, 6, 7, 0,-7,-6,-5,-4,-3,-2,-1};\n";
	}
	srcStdStr += "struct ComplexStruct {\n";
	srcStdStr += "float real;\n";
	srcStdStr += "float imag; };\n";
	srcStdStr += "typedef struct ComplexStruct SComplex;\n";

	if (d_data_type == DTYPE_PACKEDXY) {
		// In this mode, we know we have packed 4-bit in.  So we can use that to determine full scale.
		srcStdStr += "#define ONE_OVER_S4BIT_MAX 0.142857142857142857143\n";

		srcStdStr += "__kernel void CharToComplex(__global unsigned char * restrict a, __global SComplex * restrict c) {\n";
		srcStdStr += "    size_t index =  get_global_id(0);\n";
		srcStdStr += "    unsigned char tmp_char1 = a[index];\n";
		srcStdStr += "	  c[index].real = (float)twosComplementLUT[tmp_char1 >> 4] * ONE_OVER_S4BIT_MAX;\n";
		srcStdStr += "	  c[index].imag = (float)twosComplementLUT[tmp_char1 & 0x0F] * ONE_OVER_S4BIT_MAX;\n";
	}
	else {
		// In this mode, we don't know what produced the byte input, so we'll have to use full SCHAR scale
		srcStdStr += "#define ONE_OVER_SCHAR_MAX 0.007874015748031496063\n";
		srcStdStr += "__kernel void CharToComplex(__global char * restrict a, __global SComplex * restrict c) {\n";
		srcStdStr += "    size_t index =  get_global_id(0);\n";
		srcStdStr += "    size_t two_index =  index*2;\n";
		srcStdStr += "	  c[index].real = (float)a[two_index] * ONE_OVER_SCHAR_MAX;\n";
		srcStdStr += "	  c[index].imag = (float)a[two_index+1] * ONE_OVER_SCHAR_MAX;\n";
	}
	srcStdStr += "}\n";

	try {
		// Create and program from source
		if (char_to_cc_program) {
			delete char_to_cc_program;
			char_to_cc_program = NULL;
		}
		if (char_to_cc_sources) {
			delete char_to_cc_sources;
			char_to_cc_sources = NULL;
		}
		char_to_cc_sources=new cl::Program::Sources(1, std::make_pair(srcStdStr.c_str(), 0));
		char_to_cc_program = new cl::Program(*context, *char_to_cc_sources);

		// Build program
		char_to_cc_program->build(devices);

		char_to_cc_kernel=new cl::Kernel(*char_to_cc_program, (const char *)fnName.c_str());
	}
	catch(cl::Error& e) {
		std::cout << "OpenCL Error compiling kernel for " << fnName << std::endl;
		std::cout << "OpenCL Error " << e.err() << ": " << e.what() << std::endl;
		std::cout << srcStdStr << std::endl;
		exit(0);
	}

	try {
		preferredWorkGroupSizeMultiple = char_to_cc_kernel->getWorkGroupInfo<CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE>(devices[0]);
	}
	catch(cl::Error& e) {
		std::cout << "Error getting kernel preferred work group size multiple" << std::endl;
		std::cout << "OpenCL Error " << e.err() << ": " << e.what()<< std::endl;
	}

	try {
		maxWorkGroupSize = char_to_cc_kernel->getWorkGroupInfo<CL_KERNEL_WORK_GROUP_SIZE>(devices[0]);
	}
	catch(cl::Error& e) {
		std::cout << "Error getting kernel preferred work group size multiple" << std::endl;
		std::cout << "OpenCL Error " << e.err() << ": " << e.what()<< std::endl;
	}
}

int
clXEngine_impl::work_processor(int noutput_items,
		gr_vector_const_void_star &input_items,
		gr_vector_void_star &output_items, bool liveWork)
{
	gr::thread::scoped_lock guard(d_setlock);

	int items_remaining = d_integration_time - integration_tracker;

	int items_processed;

	if (noutput_items > items_remaining) {
		items_processed = items_remaining;
	}
	else {
		items_processed = noutput_items;
	}

	if (thread_process_data && ((integration_tracker+items_processed) == d_integration_time)) {
		// If a thread is already processing data and this would trigger a new one,
		// the buffer has backed up.  Let's hold and tell the engine we're not ready for this data.

		while (thread_process_data) {
			usleep(4);
		}

		// return 0;
	}

	if (!d_use_internal_synchronizer) {
		// with the internal synchronizer, we'll handle this once when we sync
		if (d_fp && !d_wrote_json) {
			// We're writing to file and we haven't written any bytes to the current file
			unsigned long lowest_tag = -1;

			for (int cur_input=0;cur_input<d_num_inputs;cur_input++) {
				std::vector<gr::tag_t> tags;
				this->get_tags_in_window(tags, cur_input, 0, noutput_items);

				int cur_tag = 0;
				int tag_size = tags.size();

				// Find the lowest tag number in this set.
				while ( cur_tag < tag_size ) {
					long tag_val = pmt::to_long(tags[cur_tag].value);
					if ( (tag_val >=0) && ( (lowest_tag == -1) || (tag_val < lowest_tag) ) ) {
						lowest_tag = tag_val;
						break;
					}
					cur_tag++;
				}

			}

			// If we found a tag, this'll write the lowest value
			if (lowest_tag >= 0) {
				write_json(lowest_tag);
			}
		}
	}

	// First we need to load the data into a matrix in the format expected by the correlator.
	// For a single polarization it's easy, we can just chain them.
	// For dual polarization, we have to interleave them so it's
	// x0r x0i y0r y0i.....

	for (int cur_block=0;cur_block<items_processed;cur_block++) {
		// For reference: 	frame_size = d_num_channels * d_num_inputs * d_npol;
		int input_start = frame_size * (integration_tracker + cur_block);

		if (d_data_type == DTYPE_BYTE) {
			input_start *= d_data_size; // interleaved IQ, so 2 data_size's at a clip
		}

		if (d_npol == 1) {
			if (d_data_type == DTYPE_BYTE) {
				for (int i=0;i<d_num_inputs;i++) {
					const char *cur_signal = (const char *) input_items[i];
					memcpy(&char_input[input_start + i*d_num_channels*d_data_size],&cur_signal[cur_block*input_size],input_size);
				}
			}
			else {
				for (int i=0;i<d_num_inputs;i++) {
					const gr_complex *cur_signal = (const gr_complex *) input_items[i];
					memcpy(&complex_input[input_start + i*d_num_channels],&cur_signal[cur_block*d_num_channels],input_size);
				}
			}
		}
		else {
			if (d_data_type == DTYPE_BYTE) {
				// We need to interleave....
				for (int i=0;i<d_num_inputs;i++) {
					const char *pol1 = (const char *) input_items[i];
					const char *pol2 = (const char *) input_items[i+d_num_inputs];

					// Each interleaved channel will now be num_channels*2 long
					// X Y X Y X Y...
					for (int k=0;k<d_num_channels;k++) {
						int input_index = input_start + i*num_chan_x2*2+k*4;
						int pol_index = cur_block*num_chan_x2+k*2;
						memcpy(&char_input[input_index],&pol1[pol_index],d_data_size);
						memcpy(&char_input[input_index+2],&pol2[pol_index],d_data_size);
					}
				}
			}
			else if (d_data_type == DTYPE_PACKEDXY) {
				// Already interleaved
				int pol_index = cur_block*num_chan_x2;
				for (int i=0;i<d_num_inputs;i++) {
					const char *pol1 = (const char *) input_items[i];
					int input_index = input_start + i*num_chan_x2;
					// Each interleaved channel will now be num_channels*2 long
					// X Y X Y X Y...
					// In packed xy mode, the input is already xyxy, just packed 4-bit in each byte.
					memcpy(&char_input[input_index],&pol1[pol_index],num_chan_x2);
				}
			}
			else {
				// We need to interleave....
				for (int i=0;i<d_num_inputs;i++) {
					const gr_complex *pol1 = (const gr_complex *) input_items[i];
					const gr_complex *pol2 = (const gr_complex *) input_items[i+d_num_inputs];

					// Each interleaved channel will now be num_channels*2 long
					// X Y X Y X Y...
					for (int k=0;k<d_num_channels;k++) {
						int input_index = input_start + i*num_chan_x2+k*2;
						int pol_index = cur_block*d_num_channels+k;
						// complex_input[input_index++] = pol1[pol_index];
						// complex_input[input_index] = pol2[pol_index];
						// The memcpy is slightly faster.
						// sizeof(gr_complex) is faster than d_data_size
						memcpy(&complex_input[input_index++],&pol1[pol_index],sizeof(gr_complex));
						memcpy(&complex_input[input_index],&pol2[pol_index],sizeof(gr_complex));
					}
				} // for i
			} // else datatype
		} // else interleave
	} // for curblock

	integration_tracker += items_processed;

	if (integration_tracker == d_integration_time) {
		// Buffer is ready for processing.
		if (!thread_process_data) {
			// thread_is_processing will only be FALSE if thread_process_data == false on the first pass,
			// in which case we don't want to send any pmt's.  Otherwise, we're in async pickup mode.
			if (thread_is_processing && (!d_output_file) && (!d_disable_output)) {
				// So this case is that we have a new block ready and the old one is complete.
				// So before transitioning to the new one, let's send the data from the last one.

				pmt::pmt_t corr_out(pmt::init_c32vector(matrix_flat_length,thread_output_matrix));
				pmt::pmt_t pdu = pmt::cons(pmt::string_to_symbol("triang_matrix"), corr_out);

				if (liveWork) {
					message_port_pub(pmt::mp("xcorr"), pdu);
				}
			}

			// Set up the pointers to the new data the thread should work with.
			thread_output_matrix = output_matrix;
			if (d_data_type == DTYPE_COMPLEX) {
				thread_complex_input = complex_input;

				// Move the current pointer to the other buffer
				if (current_write_buffer == 1) {
					// Move to buffer 2
					complex_input = complex_input2;
					output_matrix = output_matrix2;
					current_write_buffer = 2;
				}
				else {
					// Move to buffer 1
					complex_input = complex_input1;
					output_matrix = output_matrix1;
					current_write_buffer = 1;
				}
			}
			else {
				thread_char_input = char_input;

				// Move the current pointer to the other buffer
				if (current_write_buffer == 1) {
					// Move to buffer 2
					char_input = char_input2;
					output_matrix = output_matrix2;
					current_write_buffer = 2;
				}
				else {
					// Move to buffer 1
					char_input = char_input1;
					output_matrix = output_matrix1;
					current_write_buffer = 1;
				}
			}

			// Trigger the thread to process
			thread_process_data = true;
		}

		integration_tracker = 0;
	}

	// Tell runtime system how many output items we produced.
	return items_processed;
}

int
clXEngine_impl::work_test(int noutput_items,
		gr_vector_const_void_star &input_items,
		gr_vector_void_star &output_items)
{
	return work_processor(noutput_items, input_items, output_items, false);
}

int
clXEngine_impl::general_work (int noutput_items,
		gr_vector_int &ninput_items,
		gr_vector_const_void_star &input_items,
		gr_vector_void_star &output_items)
{
	if (d_use_internal_synchronizer && !d_synchronized) {
		// We need to synchronize.
		// Each timestamp will always be t[n+1] = t[n] + 16
		// Look at each input channel's tags and find the highest starting tag.
		// Take the timestamp diff for each input, and that's how many items we need to consume.
		uint64_t highest_tag = 0;
		uint64_t first_input_timestamp;

		bool test_sync = true;

		// Find the highest tag in the first slot.
		for (int cur_input=0;cur_input<d_num_inputs;cur_input++) {
			std::vector<gr::tag_t> tags;
			// We only need the first tag.  No need to get them all.
			this->get_tags_in_window(tags, cur_input, 0, 1);

			uint64_t tag0 = pmt::to_uint64(tags[0].value);

			if (cur_input == 0) {
				first_input_timestamp = tag0;
			}
			else {
				if (tag0 != first_input_timestamp) {
					test_sync = false;
				}
			}

			// Save these so we don't have to get tags and iterate again.
			tag_list[cur_input] = tag0;

			if (tag0 > highest_tag) {
				highest_tag = tag0;
			}
		}

		if (test_sync) {
			// we're actually now synchronized.  We'll set our sync flag and process as if we came in sync'd
			d_synchronized = true;
			if (d_fp && !d_wrote_json) {
					write_json(highest_tag);
			}

	        pmt::pmt_t pdu = pmt::cons( pmt::intern("synctimestamp"), pmt::from_uint64(highest_tag) );
			message_port_pub(pmt::mp("sync"),pdu);

			std::stringstream msg_stream;
			msg_stream << "Synchronized on timestamp " << highest_tag;
			GR_LOG_INFO(d_logger, msg_stream.str());
		}
		else {
			// So we're still not sync'd so we need to figure out what we need to dump.
			for (int cur_input=0;cur_input<d_num_inputs;cur_input++) {

				// tag_diff will increment by 16 with the tag #'s so no need to divide by 16.
				// We need this # anyway.
				uint64_t items_to_consume = highest_tag - tag_list[cur_input];

				if (items_to_consume > noutput_items)
					items_to_consume = noutput_items;

				consume(cur_input,items_to_consume);
			}

			// We're going to return 0 here so we don't forward any data along yet.  That won't happen till we're synchronized.
			return 0;
		}
	}

	int items_processed = work_processor(noutput_items, input_items, output_items, true);

	consume_each (items_processed);
	return items_processed;
}

void clXEngine_impl::runThread() {
	threadRunning = true;

	while (!stop_thread) {
		if (thread_process_data) {
			// This is really a one-time variable set to true on the first pass.
			thread_is_processing = true;

			if (d_data_type == DTYPE_COMPLEX) {
				xcorrelate((XComplex *)thread_complex_input, (XComplex *)thread_output_matrix);
			}
			else {
				xcorrelate(thread_char_input, (XComplex *)thread_output_matrix);
			}

			if (d_fp) {
				char *inbuf;
				inbuf = (char *)thread_output_matrix;
				long nwritten = 0;

				// If we have an open file, we're supposed to rollover files, and we're over our limit
				// reset the file.
				if ((d_fp) && (rollover_size_bytes > 0) && (d_bytesWritten >= rollover_size_bytes)) {
					close();
					open();
				}

				// Optimize write as one call
				long count = fwrite(inbuf, matrix_flat_length * sizeof(gr_complex),1,d_fp);
				if(count == 0) {
					// Error condition, nothing written for some reason.
					if(ferror(d_fp)) {
						std::cout << "[X-Engine] Write failed with error: " << std::strerror(errno) << std::endl;
					}
				}
			}

			// clear the trigger.  This will inform that data is ready.
			thread_process_data = false;
		}

		int ct = 0;

		while (!thread_process_data && (ct++ < 4) ) {
			usleep(2);
		}
	}

	threadRunning = false;
}

} /* namespace clenabled */
} /* namespace gr */

